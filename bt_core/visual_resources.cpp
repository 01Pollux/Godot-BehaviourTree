
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "visual_resources.hpp"

#if TOOLS_ENABLED
namespace behaviour_tree {
/*
... BehaviourTreeResource data

	"res://custom.json"		Path of custom nodes descriptors					= pascal string

	"Position"				position of the node								= Vector2
	"Title"					title of the node									= pascal string
	"Comment"				description of the node								= pascal string

	...
*/

void VisualBehaviourTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_set_custom_nodes_data_path", "data_path"), &VisualBehaviourTree::GDSetNodesDataPath);
	ClassDB::bind_method(D_METHOD("_get_custom_nodes_data_path"), &VisualBehaviourTree::GDGetNodesDataPath);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_nodes_data_path", PROPERTY_HINT_FILE, "*.json", PROPERTY_USAGE_EDITOR), "_set_custom_nodes_data_path", "_get_custom_nodes_data_path");

	ClassDB::bind_method(D_METHOD("_set_custom_nodes_descriptor", "descriptor"), &VisualBehaviourTree::GDSetNodesDescriptor);
	ClassDB::bind_method(D_METHOD("_get_custom_nodes_descriptor"), &VisualBehaviourTree::GDGetNodesDescriptor);
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "custom_nodes_descriptor", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_custom_nodes_descriptor", "_get_custom_nodes_descriptor");
	
	ClassDB::bind_method(D_METHOD("_set_nodes_info", "nodes"), &VisualBehaviourTree::GDSetNodesInfo);
	ClassDB::bind_method(D_METHOD("_get_nodes_info"), &VisualBehaviourTree::GDGetNodesInfo);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "bt_nodes_info", PROPERTY_HINT_ARRAY_TYPE, "", PROPERTY_USAGE_STORAGE), "_set_nodes_info", "_get_nodes_info");

	ADD_SIGNAL(MethodInfo("_on_custom_nodes_path_changed"));
}

void VisualBehaviourTree::register_types() {
	GDREGISTER_CLASS(VisualBehaviourTree);

	VBTreeResLoader.instantiate();
	VBTreeResSaver.instantiate();

	ResourceLoader::add_resource_format_loader(VBTreeResLoader);
	ResourceSaver::add_resource_format_saver(VBTreeResSaver);
}

void VisualBehaviourTree::unregister_types() {
	ResourceLoader::remove_resource_format_loader(VBTreeResLoader);
	ResourceSaver::remove_resource_format_saver(VBTreeResSaver);

	VBTreeResLoader.unref();
	VBTreeResSaver.unref();
}

Error VisualBehaviourTree::VLoadFromFile(const String &path) {
	Error err = Error::OK;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ, &err);

	err = LoadFromFile(path, file);
#if TOOLS_ENABLED
	ERR_FAIL_COND_V_MSG(err != Error::OK, err, "Failed to read visual behaviour tree from a file");
#endif

	if (err == Error::OK) {
		GDSetNodesDataPath(ReadStringFromFile(file));

		size_t size = GetNodes().size();
		m_NodesInfo.reserve(size);

		for (size_t i = 0; i < size; i++) {
			auto &node_info = m_NodesInfo.emplace_back();

			node_info.Position.x = file->get_float();
			node_info.Position.y = file->get_float();
			node_info.Title = ReadStringFromFile(file);
			node_info.Comment = ReadStringFromFile(file);
		}
	}
	return err;
}

Error VisualBehaviourTree::VSaveToFile(const String &path) {
	Error err = Error::OK;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE, &err);

#if TOOLS_ENABLED
	ERR_FAIL_COND_V_MSG(err != Error::OK, err, "Failed to write visual behaviour tree to a file of path: " + path);
#endif

	SaveToFile(path, file);

	if (path.ends_with(".vbtree")) {
		if (err == Error::OK) {
			WriteStringToFile(file, GDGetNodesDataPath());
#if TOOLS_ENABLED
			ERR_FAIL_COND_V(GetNodes().size() != m_NodesInfo.size(), Error::ERR_FILE_CORRUPT);
#endif

			for (auto &node_info : m_NodesInfo) {
				file->store_float(node_info.Position.x);
				file->store_float(node_info.Position.y);
				WriteStringToFile(file, node_info.Title);
				WriteStringToFile(file, node_info.Comment);
			}
		}
	}

	return err;
}

void VisualBehaviourTree::GDSetNodesDataPath(const String &file_path) {
	if (!file_path.is_empty()) {
		if (m_CustomNodesDescriptor.has("_path") && m_CustomNodesDescriptor["_path"] == file_path)
			return;

		String file_data = FileAccess::get_file_as_string(file_path);
		if (!file_data.is_empty()) {
			JSON json;
			Error err = json.parse(file_data);
#if TOOLS_ENABLED
			ERR_FAIL_COND_MSG(err != Error::OK, "Failed to parse file as JSON");
#endif
			m_CustomNodesDescriptor = json.get_data();
		}

		m_CustomNodesDescriptor["_path"] = file_path;
		emit_signal("_on_custom_nodes_path_changed");
	}
}

String VisualBehaviourTree::GDGetNodesDataPath() const {
	return m_CustomNodesDescriptor.has("_path") ? m_CustomNodesDescriptor["_path"] : "";
}


Ref<Resource> ResourceFormatLoaderVBehaviourTree::load(
		const String &p_path,
		const String &p_original_path,
		Error *r_error,
		bool p_use_sub_threads,
		float *r_progress,
		CacheMode p_cache_mode) {
	Ref<VisualBehaviourTree> res;
	res.instantiate();

	if (r_error)
		*r_error = OK;

	Error err = res->VLoadFromFile(p_path);
	if (r_error)
		*r_error = err;

	return res;
}

void ResourceFormatLoaderVBehaviourTree::get_recognized_extensions(List<String> *r_extensions) const {
	r_extensions->push_back("vbtree");
}

bool ResourceFormatLoaderVBehaviourTree::handles_type(const String &p_type) const {
	return p_type == "VisualBehaviourTree";
}

String ResourceFormatLoaderVBehaviourTree::get_resource_type(const String &p_path) const {
	return p_path.get_extension().to_lower() == "vbtree" ? "VisualBehaviourTree" : "";
}


Error ResourceFormatSaverVBehaviourTree::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<VisualBehaviourTree> btree = p_resource;
	return btree->VSaveToFile(p_path);
}

bool ResourceFormatSaverVBehaviourTree::recognize(const Ref<Resource> &p_resource) const {
	return Object::cast_to<VisualBehaviourTree>(*p_resource) != nullptr;
}

void ResourceFormatSaverVBehaviourTree::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *r_extensions) const {
	r_extensions->clear();
	r_extensions->push_back("vbtree");
	r_extensions->push_back("btree");
}
} //namespace behaviour_tree
#endif
