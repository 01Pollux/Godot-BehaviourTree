
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>

#include "resources.hpp"
#include "tree.hpp"

#include "action_node.hpp"
#include "decorator_node.hpp"
#include "composite_node.hpp"

namespace behaviour_tree
{
	/*
	0 | 1						_always_running										= 1 byte
	X							index of root node, -1 if it doesn't exists			= 2 byte
	X							number of nodes										= 2 bytes

		"SequenceNode"			name of node										= pascal string
		"res://script.gd"		path of the script									= pascal string
		X ...					indices of childrens, -1 indicates the end of array	= 2 bytes each
		X						number of keyvalue for data							= 2 bytes
		String -- String		pair of key and value for data						= pascal string

		...
	*/
	void BehaviourTreeResource::_bind_methods()
	{
		ClassDB::bind_method(D_METHOD("set_behaviour_tree", "tree"), &BehaviourTreeResource::SetTree);
		ClassDB::bind_method(D_METHOD("get_behaviour_tree"), &BehaviourTreeResource::GetTree);
	}

	void BehaviourTreeResource::register_types()
	{
		GDREGISTER_CLASS(BehaviourTreeResource);

		BTreeResLoader.instantiate();
		BTreeResSaver.instantiate();

		ResourceLoader::add_resource_format_loader(BTreeResLoader);
		ResourceSaver::add_resource_format_saver(BTreeResSaver);
	}

	void BehaviourTreeResource::unregister_types()
	{
		ResourceLoader::remove_resource_format_loader(BTreeResLoader);
		ResourceSaver::remove_resource_format_saver(BTreeResSaver);

		BTreeResLoader.unref();
		BTreeResSaver.unref();
	}


	void BehaviourTreeResource::SetTree(Ref<BehaviourTree> res) noexcept
	{
		m_Tree = res;
		res.unref();
	}


	Error BehaviourTreeResource::LoadFromFile(const String& path, Ref<FileAccess> file)
	{
		Error err = Error::OK;
		if (file.is_null())
			file = FileAccess::open(path, FileAccess::READ, &err);

		if (err != Error::OK || file->eof_reached())
			return err;
		try
		{
			bool always_running = file->get_8();
			size_t root_node_index = file->get_16();

			NodeLoadInfoContainer loaded_nodes = LoadNodesFromFile(file);
			ResolveNodesChildrensFromFile(loaded_nodes);

			m_Tree.instantiate();
			auto& tree_nodes = m_Tree->GetNodes();

			if (!loaded_nodes.empty())
			{
				if (root_node_index != std::numeric_limits<uint16_t>::max())
					m_Tree->SetRootNode(loaded_nodes[root_node_index].Node);

				tree_nodes.reserve(loaded_nodes.size());
				for (auto& node : loaded_nodes)
					tree_nodes.emplace_back(node.Node);
			}

			m_Tree->SetAlwaysRunning(always_running);
		}
		catch (const std::exception& ex)
		{
			ERR_PRINT(ex.what());
			err = file->get_error();
		}

		return err;
	}

	Error BehaviourTreeResource::SaveToFile(const String& path, Ref<FileAccess> file)
	{
		Error err = Error::OK;
		if (file.is_null())
			file = FileAccess::open(path, FileAccess::WRITE, &err);

		if (err == Error::OK)
		{
			file->store_8(m_Tree->IsAlwaysRunning() ? 1 : 0);

			NodeLoadInfoContainer loaded_nodes = TreeToNodesLoadInfo();
			ResolveNodesIndicesForFile(loaded_nodes);

			SaveNodesToFile(loaded_nodes, m_Tree->GetRootNode(), file);
		}
		return err;
	}


	auto BehaviourTreeResource::LoadNodesFromFile(Ref<FileAccess>& file) ->
		NodeLoadInfoContainer
	{
		uint16_t number_of_nodes = file->get_16();

		NodeLoadInfoContainer loaded_nodes;
		loaded_nodes.reserve(static_cast<size_t>(number_of_nodes));
		loaded_nodes.reserve(static_cast<size_t>(number_of_nodes));

		for (uint16_t i = 0; i < number_of_nodes; i++)
		{
			String node_cls_name = ReadStringFromFile(file);
			String node_script = ReadStringFromFile(file);

			Object* object = ClassDB::instantiate(node_cls_name);
			ERR_CONTINUE_MSG(object == nullptr, "Invalid node name/script");

			Ref<IBehaviourTreeNodeBehaviour> node = Object::cast_to<IBehaviourTreeNodeBehaviour>(object);;
			ERR_CONTINUE_MSG(node == nullptr, "Node is not of type IBehaviourTreeNodeBehaviour");

			if (!node_script.is_empty())
			{
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
			while (true)
			{
				idx = file->get_16();
				if (idx == std::numeric_limits<uint16_t>::max())
					break;

				switch (type)
				{
				case NodeType::Decorator:
				case NodeType::Composite:
				{
					info.Indices.push_back(idx);
					break;
				}
				}
			}

			uint16_t data_count = file->get_16();
			Dictionary dict;

			for (uint16_t j = 0; j < data_count; j++)
			{
				String key = ReadStringFromFile(file);
				String value = ReadStringFromFile(file);
				dict[key] = value;
			}

			if (data_count != 0)
				node->DeserializeNode(dict);

			loaded_nodes.emplace_back(std::move(info));
		}

		return loaded_nodes;
	}

	void BehaviourTreeResource::SaveNodesToFile(const NodeLoadInfoContainer& loaded_nodes, IBehaviourTreeNodeBehaviour* root_node, Ref<FileAccess>& file)
	{
		uint16_t root_node_index = std::numeric_limits<uint16_t>::max();
		for (size_t i = 0; i < loaded_nodes.size(); i++)
		{
			if (loaded_nodes[i].Node == root_node)
			{
				root_node_index = static_cast<uint16_t>(i);
				break;
			}
		}

		file->store_16(root_node_index);
		file->store_16(static_cast<uint16_t>(loaded_nodes.size()));

		for (auto& cur_node : loaded_nodes)
		{
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

			file->store_16(static_cast<uint16_t>(out_data.size()));

			if (!out_data.is_empty())
			{
				auto keys = out_data.keys();
				auto values = out_data.values();

				for (int i = 0; i < keys.size(); i++)
				{
					WriteStringToFile(file, keys[i]);
					WriteStringToFile(file, values[i]);
				}
			}
		}
	}

	void BehaviourTreeResource::ResolveNodesChildrensFromFile(NodeLoadInfoContainer& loaded_nodes)
	{
		for (auto node_info = loaded_nodes.begin(); node_info != loaded_nodes.end(); node_info++)
		{
			if (node_info->Indices.empty())
				continue;
			switch (node_info->Type)
			{
			case NodeType::Decorator:
			{
				int node_index = node_info->Indices[0];
				ERR_CONTINUE_MSG(node_index >= static_cast<int>(loaded_nodes.size()), "Index out of bounds for behaviour tree");

				if (node_index != -1)
				{
					IBehaviourTreeNodeBehaviour* child = *loaded_nodes[node_index].Node;
					IBehaviourTreeDecoratorNode* parent = Object::cast_to<IBehaviourTreeDecoratorNode>(*node_info->Node);

					parent->SetChild(child);
				}
				break;
			}
			case NodeType::Composite:
			{
				IBehaviourTreeCompositeNode* parent = Object::cast_to<IBehaviourTreeCompositeNode>(*node_info->Node);
				for (int node_index : node_info->Indices)
				{
					ERR_CONTINUE_MSG(node_index >= static_cast<int>(loaded_nodes.size()), "Index out of bounds for behaviour tree");
					if (node_index != -1)
					{
						IBehaviourTreeNodeBehaviour* child = *loaded_nodes[node_index].Node;
						parent->AddChild(child);
					}
				}
				break;
			}
			}
		}
	}

	void BehaviourTreeResource::ResolveNodesIndicesForFile(NodeLoadInfoContainer& loaded_nodes)
	{
		for (size_t i = 0; i < loaded_nodes.size(); i++)
		{
			auto& cur_node = loaded_nodes[i];

			switch (cur_node.Type)
			{
			case NodeType::Decorator:
			case NodeType::Composite:
			{
				std::vector<IBehaviourTreeNodeBehaviour*> childrens;
				cur_node.Node->GetChildrens(childrens);

				for (IBehaviourTreeNodeBehaviour* child : childrens)
				{
					for (size_t j = 0; j < loaded_nodes.size(); j++)
					{
						if (loaded_nodes[j].Node == child)
						{
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


	auto BehaviourTreeResource::TreeToNodesLoadInfo() ->
		NodeLoadInfoContainer
	{
		NodeLoadInfoContainer loaded_nodes;
		auto& tree_nodes = m_Tree->GetNodes();

		if (!tree_nodes.empty())
		{
			loaded_nodes.reserve(tree_nodes.size());
			for (auto& cur_node : tree_nodes)
			{
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
	}



	void BehaviourTreeResource::WriteStringToFile(Ref<FileAccess>& file, const String& key)
	{
		if (key.is_empty())
		{
			file->store_32(0);
			file->store_8(0);
		}
		else
			file->store_pascal_string(key);
	}


	String BehaviourTreeResource::ReadStringFromFile(Ref<FileAccess>& file)
	{
		uint32_t len = file->get_32();
		String str = "";

		if (!len)
		{
			[[maybe_unused]] char dummy = file->get_8();
		}
		else
		{
			CharString cs;
			cs.resize(len + 1);
			file->get_buffer((uint8_t*)cs.ptr(), len);
			cs[len] = 0;

			str.parse_utf8(cs.ptr(), cs.size());
		}
		return str;
	}


	Ref<Resource> ResourceFormatLoaderBehaviourTree::load(
		const String& p_path,
		const String& p_original_path,
		Error* r_error,
		bool p_use_sub_threads,
		float* r_progress,
		CacheMode p_cache_mode
	)
	{
		Ref<BehaviourTreeResource> res;
		res.instantiate();

		if (r_error)
			*r_error = OK;

		Error err = res->LoadFromFile(p_path);
		if (r_error)
			*r_error = err;

		return res;
	}

	void ResourceFormatLoaderBehaviourTree::get_recognized_extensions(List<String>* r_extensions) const
	{
		if (!r_extensions->find("btree"))
			r_extensions->push_back("btree");
	}

	bool ResourceFormatLoaderBehaviourTree::handles_type(const String& p_type) const
	{
		return p_type == "BehaviourTreeResource";
	}

	String ResourceFormatLoaderBehaviourTree::get_resource_type(const String& p_path) const
	{
		return p_path.get_extension().to_lower() == "btree" ? "BehaviourTreeResource" : "";
	}


	Error ResourceFormatSaverBehaviourTree::save(const String& p_path, const Ref<Resource>& p_resource, uint32_t p_flags)
	{
		Ref<BehaviourTreeResource> btree = p_resource;
		return btree->SaveToFile(p_path);
	}

	bool ResourceFormatSaverBehaviourTree::recognize(const Ref<Resource>& p_resource) const
	{
		return p_resource->get_class_name() == "BehaviourTreeResource";
	}

	void ResourceFormatSaverBehaviourTree::get_recognized_extensions(const Ref<Resource>& p_resource, List<String>* r_extensions) const
	{
		if (Object::cast_to<BehaviourTreeResource>(*p_resource))
			r_extensions->push_back("btree");
	}
}
