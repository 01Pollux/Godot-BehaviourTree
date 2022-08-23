
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include "tree.hpp"

#include "action_node.hpp"
#include "composite_node.hpp"
#include "decorator_node.hpp"

namespace behaviour_tree {
/*
0 | 1						_always_running										= 1 byte
X							index of root node, -1 if it doesn't exists			= 2 byte
X							number of nodes										= 2 bytes

	"SequenceNode"			name of node										= pascal string
	"res://script.gd"		path of the script									= pascal string
	X ...					indices of childrens, -1 indicates the end of array	= 2 bytes each
	X						number of keyvalue for data							= 4 bytes
	Variant -- Variant		pair of key and value for data						= variants

	...
*/
struct NodeLoadInfo {
	Ref<IBehaviourTreeNodeBehaviour> Node;
	NodeType Type;
	std::vector<uint16_t> Indices;
};

using NodeLoadInfoContainer = std::vector<NodeLoadInfo>;

static NodeLoadInfoContainer LoadNodesFromFile(Ref<FileAccess> &file);
static void SaveNodesToFile(const NodeLoadInfoContainer &loaded_nodes, Ref<FileAccess> &file);

static void ResolveNodesChildrensFromFile(NodeLoadInfoContainer &nodes);
static void ResolveNodesIndicesForFile(NodeLoadInfoContainer &nodes);

Error BehaviourTree::LoadFromFile(const String &path, Ref<FileAccess> file) {
	Error err = Error::OK;
	if (file.is_null())
		file = FileAccess::open(path, FileAccess::READ, &err);

	if (err != Error::OK || file->eof_reached()) {
		return err;
	}
	try {
		bool always_running = file->get_8();
		uint16_t root_node_index = file->get_16();

		NodeLoadInfoContainer loaded_nodes = LoadNodesFromFile(file);
		ResolveNodesChildrensFromFile(loaded_nodes);

		auto &tree_nodes = GetNodes();
		if (!loaded_nodes.empty()) {
			if (root_node_index != std::numeric_limits<uint16_t>::max())
				GDSetRootNodeIndex(root_node_index);

			tree_nodes.reserve(loaded_nodes.size());
			for (auto &node : loaded_nodes)
				tree_nodes.emplace_back(node.Node);
		}

		SetAlwaysRunning(always_running);
#if TOOLS_ENABLED
	} catch (const std::exception &ex) {
		ERR_PRINT(ex.what());
#else
	} catch (...) {

#endif
		err = file->get_error();
	}

	return err;
}

Error BehaviourTree::SaveToFile(const String &path, Ref<FileAccess> file) {
	Error err = Error::OK;
	if (file.is_null())
		file = FileAccess::open(path, FileAccess::WRITE, &err);

	if (err == Error::OK) {
		file->store_8(IsAlwaysRunning() ? 1 : 0);

		auto tree_to_nodes_load_info = [this]() -> NodeLoadInfoContainer {
			NodeLoadInfoContainer loaded_nodes;
			auto &tree_nodes = GetNodes();

			if (!tree_nodes.empty()) {
				loaded_nodes.reserve(tree_nodes.size());
				for (auto &cur_node : tree_nodes) {
					NodeLoadInfo info{};
					info.Node = cur_node;

					if (Object::cast_to<IBehaviourTreeDecoratorNode>(*cur_node))
						info.Type = NodeType::Decorator;
					else if (Object::cast_to<IBehaviourTreeCompositeNode>(*cur_node))
						info.Type = NodeType::Composite;
					else
						info.Type = NodeType::Action;

					loaded_nodes.emplace_back(std::move(info));
				}
			}

			return loaded_nodes;
		};

		NodeLoadInfoContainer loaded_nodes = tree_to_nodes_load_info();
		ResolveNodesIndicesForFile(loaded_nodes);

		file->store_16(static_cast<uint16_t>(GDGetRootNodeIndex()));
		SaveNodesToFile(loaded_nodes, file);
	}
	return err;
}

Ref<Resource> ResourceFormatLoaderBehaviourTree::load(
		const String &p_path,
		const String &p_original_path,
		Error *r_error,
		bool p_use_sub_threads,
		float *r_progress,
		CacheMode p_cache_mode) {
	Ref<BehaviourTree> res;
	res.instantiate();

	if (r_error)
		*r_error = OK;

	Error err = res->LoadFromFile(p_path);
	if (r_error)
		*r_error = err;

	return res;
}

void ResourceFormatLoaderBehaviourTree::get_recognized_extensions(List<String> *r_extensions) const {
	r_extensions->push_back("btree");
}

bool ResourceFormatLoaderBehaviourTree::handles_type(const String &p_type) const {
	return p_type == "BehaviourTree";
}

String ResourceFormatLoaderBehaviourTree::get_resource_type(const String &p_path) const {
	return p_path.get_extension().to_lower() == "btree" ? "BehaviourTree" : "";
}

Error ResourceFormatSaverBehaviourTree::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<BehaviourTree> btree = p_resource;
	return btree->SaveToFile(p_path);
}

bool ResourceFormatSaverBehaviourTree::recognize(const Ref<Resource> &p_resource) const {
	return Object::cast_to<BehaviourTree>(*p_resource) != nullptr;
}

void ResourceFormatSaverBehaviourTree::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *r_extensions) const {
	r_extensions->clear();
	r_extensions->push_back("btree");
}

NodeLoadInfoContainer LoadNodesFromFile(Ref<FileAccess> &file) {
	uint16_t number_of_nodes = file->get_16();

	NodeLoadInfoContainer loaded_nodes;
	loaded_nodes.reserve(static_cast<size_t>(number_of_nodes));
	loaded_nodes.reserve(static_cast<size_t>(number_of_nodes));

	for (uint16_t i = 0; i < number_of_nodes; i++) {
		String node_cls_name = ReadStringFromFile(file);
		String node_script = ReadStringFromFile(file);

		Object *object = ClassDB::instantiate(node_cls_name);
		ERR_CONTINUE_MSG(object == nullptr, "Invalid node name/script");

		Ref<IBehaviourTreeNodeBehaviour> node = Object::cast_to<IBehaviourTreeNodeBehaviour>(object);

		ERR_CONTINUE_MSG(node == nullptr, "Node is not of type IBehaviourTreeNodeBehaviour");

		if (!node_script.is_empty()) {
			Ref<Script> script = ResourceLoader::load(node_script);
			if (script.is_valid())
				object->set_script(script);
		}

		NodeType type = NodeType::Action;
		if (Object::cast_to<IBehaviourTreeDecoratorNode>(*node))
			type = NodeType::Decorator;
		else if (Object::cast_to<IBehaviourTreeCompositeNode>(*node))
			type = NodeType::Composite;

		NodeLoadInfo info{};
		info.Node = node;
		info.Type = type;

		uint16_t idx;
		while (true) {
			idx = file->get_16();
			if (idx == std::numeric_limits<uint16_t>::max())
				break;

			switch (type) {
				case NodeType::Decorator:
				case NodeType::Composite: {
					info.Indices.push_back(idx);
					break;
				}
			}
		}

		Dictionary dict;
		{
			Variant var = ReadVariantFromFile(file);
			if (var.get_type() == Variant::DICTIONARY)
				dict = var;
		}

		if (!dict.is_empty())
			node->DeserializeNode(dict);

		loaded_nodes.emplace_back(std::move(info));
	}

	return loaded_nodes;
}

void SaveNodesToFile(const NodeLoadInfoContainer &loaded_nodes, Ref<FileAccess> &file) {
	file->store_16(static_cast<uint16_t>(loaded_nodes.size()));

	for (auto &cur_node : loaded_nodes) {
		WriteStringToFile(file, cur_node.Node->get_class_name());
		Ref<Script> script = cur_node.Node->get_script();
		String file_path;
		if (script.is_valid())
			file_path = script->get_path();
		WriteStringToFile(file, file_path);

		for (uint16_t idx : cur_node.Indices)
			file->store_16(idx);
		file->store_16(std::numeric_limits<uint16_t>::max());

		Dictionary out_data;
		cur_node.Node->SerializeNode(out_data);
		WriteVariantToFile(file, out_data);
	}
}

void ResolveNodesChildrensFromFile(NodeLoadInfoContainer &loaded_nodes) {
	for (auto node_info = loaded_nodes.begin(); node_info != loaded_nodes.end(); node_info++) {
		if (node_info->Indices.empty())
			continue;
		switch (node_info->Type) {
			case NodeType::Decorator: {
				int node_index = node_info->Indices[0];
				ERR_CONTINUE_MSG(node_index >= static_cast<int>(loaded_nodes.size()), "Index out of bounds for behaviour tree");

				if (node_index != -1) {
					IBehaviourTreeNodeBehaviour *child = *loaded_nodes[node_index].Node;
					IBehaviourTreeDecoratorNode *parent = Object::cast_to<IBehaviourTreeDecoratorNode>(*node_info->Node);

					parent->SetChild(child);
				}
				break;
			}
			case NodeType::Composite: {
				IBehaviourTreeCompositeNode *parent = Object::cast_to<IBehaviourTreeCompositeNode>(*node_info->Node);
				for (int node_index : node_info->Indices) {
					ERR_CONTINUE_MSG(node_index >= static_cast<int>(loaded_nodes.size()), "Index out of bounds for behaviour tree");
					if (node_index != -1) {
						IBehaviourTreeNodeBehaviour *child = *loaded_nodes[node_index].Node;
						parent->AddChild(child);
					}
				}
				break;
			}
		}
	}
}

void ResolveNodesIndicesForFile(NodeLoadInfoContainer &loaded_nodes) {
	for (size_t i = 0; i < loaded_nodes.size(); i++) {
		auto &cur_node = loaded_nodes[i];

		switch (cur_node.Type) {
			case NodeType::Decorator:
			case NodeType::Composite: {
				std::vector<IBehaviourTreeNodeBehaviour *> childrens;
				cur_node.Node->GetChildrens(childrens);

				for (IBehaviourTreeNodeBehaviour *child : childrens) {
					for (size_t j = 0; j < loaded_nodes.size(); j++) {
						if (loaded_nodes[j].Node == child) {
							cur_node.Indices.push_back(j);
							break;
						}
					}
				}
				break;
			}
		}
	}
}

void WriteStringToFile(Ref<FileAccess> &file, const String &key) {
	if (key.is_empty()) {
		file->store_32(0);
		file->store_8(0);
	} else
		file->store_pascal_string(key);
}

void WriteVariantToFile(Ref<FileAccess> &file, const Variant &var) {
	file->store_8(var.get_type());
	switch (var.get_type()) {
		case Variant::BOOL:
			file->store_8(var.operator bool());
			break;
		case Variant::INT:
			file->store_32(var.operator int());
			break;
		case Variant::FLOAT:
			file->store_float(var);
			break;
		case Variant::STRING:
			WriteStringToFile(file, var);
			break;

		case Variant::VECTOR2: {
			Vector2 val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::VECTOR2I: {
			Vector2i val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::RECT2: {
			Rect2 val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::RECT2I: {
			Rect2i val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::VECTOR3: {
			Vector3 val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::VECTOR3I: {
			Vector3i val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::TRANSFORM2D: {
			Transform2D val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::VECTOR4: {
			Vector4 val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::VECTOR4I: {
			Vector4i val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::PLANE: {
			Plane val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::QUATERNION: {
			Quaternion val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::AABB: {
			AABB val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::BASIS: {
			Basis val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::TRANSFORM3D: {
			Transform3D val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::PROJECTION: {
			Projection val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}

		// misc types
		case Variant::COLOR: {
			Color color = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&color), sizeof(color));
			break;
		}
		case Variant::STRING_NAME: {
			WriteStringToFile(file, var);
			break;
		}
		case Variant::NODE_PATH: {
			NodePath val = var;
			WriteStringToFile(file, val);
			break;
		}
		case Variant::RID: {
			RID val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::OBJECT: {
			Object *val = var;
			file->store_64(val->get_instance_id());
			break;
		}
		case Variant::CALLABLE: {
			Callable val = var;
			ERR_FAIL_COND_MSG(val.is_custom() || val.is_null(), "Invalid Callable passed in Variant");
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::SIGNAL: {
			Signal val = var;
			file->store_buffer(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
			break;
		}
		case Variant::DICTIONARY: {
			Dictionary val = var;
			auto keys = val.keys();
			auto vals = val.values();
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++) {
				WriteVariantToFile(file, keys[i]);
				WriteVariantToFile(file, vals[i]);
			}
			break;
		}
		case Variant::ARRAY: {
			Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				WriteVariantToFile(file, val[i]);
			break;
		}

		// typed arrays
		case Variant::PACKED_BYTE_ARRAY: {
			PackedByteArray val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				file->store_8(val[i]);
			break;
		}
		case Variant::PACKED_INT32_ARRAY: {
			PackedInt32Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				file->store_32(val[i]);
			break;
		}
		case Variant::PACKED_INT64_ARRAY: {
			PackedInt64Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				file->store_64(val[i]);
			break;
		}
		case Variant::PACKED_FLOAT32_ARRAY: {
			PackedFloat32Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				file->store_float(val[i]);
			break;
		}
		case Variant::PACKED_FLOAT64_ARRAY: {
			PackedFloat64Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				file->store_double(val[i]);
			break;
		}
		case Variant::PACKED_STRING_ARRAY: {
			PackedStringArray val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++)
				WriteStringToFile(file, val[i]);
			break;
		}
		case Variant::PACKED_VECTOR2_ARRAY: {
			PackedVector2Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++) {
				file->store_float(val[i].x);
				file->store_float(val[i].y);
			}
			break;
		}
		case Variant::PACKED_VECTOR3_ARRAY: {
			PackedVector3Array val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++) {
				file->store_float(val[i].x);
				file->store_float(val[i].y);
				file->store_float(val[i].z);
			}
			break;
		}
		case Variant::PACKED_COLOR_ARRAY: {
			PackedColorArray val = var;
			file->store_32(val.size());
			for (int i = 0; i < val.size(); i++) {
				file->store_float(val[i].r);
				file->store_float(val[i].g);
				file->store_float(val[i].b);
				file->store_float(val[i].a);
			}
			break;
		}

		default: {
			ERR_FAIL_COND_MSG(var.get_type() == Variant::NIL || var.is_null(), "Invalid variant in Behaviour Tree");
		}
	}
}

String ReadStringFromFile(Ref<FileAccess> &file) {
	uint32_t len = file->get_32();
	String str = "";

	if (!len) {
		[[maybe_unused]] char dummy = file->get_8();
	} else {
		CharString cs;
		cs.resize(len + 1);
		file->get_buffer((uint8_t *)cs.ptr(), len);
		cs[len] = 0;

		str.parse_utf8(cs.ptr(), cs.size());
	}
	return str;
}

Variant ReadVariantFromFile(Ref<FileAccess> &file) {
	switch (file->get_8()) {
		case Variant::BOOL:
			return static_cast<bool>(file->get_8());
			break;
		case Variant::INT:
			return static_cast<int>(file->get_32());
			break;
		case Variant::FLOAT:
			return static_cast<float>(file->get_float());
			break;
		case Variant::STRING:
			return ReadStringFromFile(file);
			break;

		case Variant::VECTOR2: {
			Vector2 val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::VECTOR2I: {
			Vector2i val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::RECT2: {
			Rect2 val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::RECT2I: {
			Rect2i val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::VECTOR3: {
			Vector3 val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::VECTOR3I: {
			Vector3i val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::TRANSFORM2D: {
			Transform2D val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::VECTOR4: {
			Vector4 val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::VECTOR4I: {
			Vector4i val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::PLANE: {
			Plane val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::QUATERNION: {
			Quaternion val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::AABB: {
			AABB val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::BASIS: {
			Basis val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::TRANSFORM3D: {
			Transform3D val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::PROJECTION: {
			Projection val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}

		// misc types
		case Variant::COLOR: {
			Color val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::STRING_NAME: {
			return ReadStringFromFile(file);
			break;
		}
		case Variant::NODE_PATH: {
			return NodePath(ReadStringFromFile(file));
		}
		case Variant::RID: {
			RID val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::OBJECT: {
			Object *val = ObjectDB::get_instance(ObjectID(file->get_64()));
			if (!val)
				ERR_PRINT("Invalid Object passed in Variant");
			return val;
		}
		case Variant::CALLABLE: {
			Callable val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			if (val.is_custom() || val.is_null())
				ERR_PRINT("Invalid Callable passed in Variant");
			return val;
		}
		case Variant::SIGNAL: {
			Signal val;
			file->get_buffer(reinterpret_cast<uint8_t *>(&val), sizeof(val));
			return val;
		}
		case Variant::DICTIONARY: {
			Dictionary val;
			int size = file->get_32();
			for (int i = 0; i < size; i++) {
				Variant key = ReadVariantFromFile(file);
				Variant value = ReadVariantFromFile(file);
				val[key] = value;
			}
			return val;
		}
		case Variant::ARRAY: {
			Array val;
			int size = file->get_32();
			val.resize(size);

			for (int i = 0; i < size; i++)
				val[i] = ReadVariantFromFile(file);
			return val;
		}

		// typed arrays
		case Variant::PACKED_BYTE_ARRAY: {
			PackedByteArray val;
			int size = file->get_32();

			for (int i = 0; i < size; i++)
				val.append(file->get_8());

			return val;
		}
		case Variant::PACKED_INT32_ARRAY: {
			PackedInt32Array val;
			int size = file->get_32();

			for (int i = 0; i < size; i++)
				val.append(file->get_32());

			return val;
		}
		case Variant::PACKED_INT64_ARRAY: {
			PackedInt64Array val;
			int size = file->get_32();

			for (int i = 0; i < size; i++)
				val.append(file->get_64());

			return val;
		}
		case Variant::PACKED_FLOAT32_ARRAY: {
			PackedFloat32Array val;
			int size = file->get_32();

			for (int i = 0; i < size; i++)
				val.append(file->get_float());

			return val;
		}
		case Variant::PACKED_FLOAT64_ARRAY: {
			PackedFloat64Array val;
			int size = file->get_32();

			for (int i = 0; i < size; i++)
				val.append(file->get_double());

			return val;
		}
		case Variant::PACKED_STRING_ARRAY: {
			PackedStringArray val;
			int size = file->get_32();

			for (int i = 0; i < size; i++)
				val.append(ReadStringFromFile(file));

			return val;
		}
		case Variant::PACKED_VECTOR2_ARRAY: {
			PackedVector2Array val;
			int size = file->get_32();

			for (int i = 0; i < size; i++) {
				Vector2 vec;
				vec.x = file->get_float();
				vec.y = file->get_float();
				val.append(vec);
			}

			return val;
		}
		case Variant::PACKED_VECTOR3_ARRAY: {
			PackedVector3Array val;
			int size = file->get_32();

			for (int i = 0; i < size; i++) {
				Vector3 vec;
				vec.x = file->get_float();
				vec.y = file->get_float();
				vec.z = file->get_float();
				val.append(vec);
			}

			return val;
		}
		case Variant::PACKED_COLOR_ARRAY: {
			PackedColorArray val;
			int size = file->get_32();

			for (int i = 0; i < size; i++) {
				Color color;
				color.r = file->get_float();
				color.g = file->get_float();
				color.b = file->get_float();
				color.a = file->get_float();
				val.append(color);
			}

			return val;
		}

		default: {
			ERR_PRINT("Invalid variant in Behaviour Tree");
			return Variant{};
		}
	}
}
} //namespace behaviour_tree
