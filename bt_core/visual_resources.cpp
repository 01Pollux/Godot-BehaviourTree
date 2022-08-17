
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

void VBehaviourTreeResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_set_custom_nodes_data_path", "data_path"), &VBehaviourTreeResource::SetNodesDataPath);
	ClassDB::bind_method(D_METHOD("_get_custom_nodes_data_path"), &VBehaviourTreeResource::GetNodesDataPath);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_nodes_data_path", PROPERTY_HINT_FILE, "*.json"), "_set_custom_nodes_data_path", "_get_custom_nodes_data_path");

	ADD_SIGNAL(MethodInfo("_on_custom_nodes_path_changed"));
}

void VBehaviourTreeResource::register_types() {
	GDREGISTER_CLASS(VBehaviourTreeResource);

	VBTreeResLoader.instantiate();
	VBTreeResSaver.instantiate();

	ResourceLoader::add_resource_format_loader(VBTreeResLoader);
	ResourceSaver::add_resource_format_saver(VBTreeResSaver);
}

void VBehaviourTreeResource::unregister_types() {
	ResourceLoader::remove_resource_format_loader(VBTreeResLoader);
	ResourceSaver::remove_resource_format_saver(VBTreeResSaver);

	VBTreeResLoader.unref();
	VBTreeResSaver.unref();
}

Error VBehaviourTreeResource::VLoadFromFile(const String &path) {
	Error err = Error::OK;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ, &err);

	err = LoadFromFile(path, file);
#if TOOLS_ENABLED
	ERR_FAIL_COND_V_MSG(err != Error::OK, err, "Failed to read visual behaviour tree from a file");
#endif

	if (err == Error::OK) {
		SetNodesDataPath(ReadStringFromFile(file));

		size_t size = GetTree()->GetNodes().size();
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

Error VBehaviourTreeResource::VSaveToFile(const String &path) {
	Error err = Error::OK;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE, &err);

#if TOOLS_ENABLED
	ERR_FAIL_COND_V_MSG(err != Error::OK, err, "Failed to write visual behaviour tree to a file of path: " + path);
#endif

	SaveToFile(path, file);

	if (path.ends_with(".vbtree")) {
		if (err == Error::OK) {
			WriteStringToFile(file, GetNodesDataPath());
#if TOOLS_ENABLED
			ERR_FAIL_COND_V(GetTree()->GetNodes().size() != m_NodesInfo.size(), Error::ERR_FILE_CORRUPT);
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

void VBehaviourTreeResource::SetNodesDataPath(const String &file_path) {
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

String VBehaviourTreeResource::GetNodesDataPath() const {
	return m_CustomNodesDescriptor.has("_path") ? m_CustomNodesDescriptor["_path"] : "";
}

Ref<Resource> ResourceFormatLoaderVBehaviourTree::load(
		const String &p_path,
		const String &p_original_path,
		Error *r_error,
		bool p_use_sub_threads,
		float *r_progress,
		CacheMode p_cache_mode) {
	Ref<VBehaviourTreeResource> res;
	res.instantiate();

	if (r_error)
		*r_error = OK;

	Error err = res->VLoadFromFile(p_path);
	if (r_error)
		*r_error = err;

	return res;
}

void ResourceFormatLoaderVBehaviourTree::get_recognized_extensions(List<String> *r_extensions) const {
	if (!r_extensions->find("vbtree"))
		r_extensions->push_back("vbtree");
}

bool ResourceFormatLoaderVBehaviourTree::handles_type(const String &p_type) const {
	return p_type == "VBehaviourTreeResource";
}

String ResourceFormatLoaderVBehaviourTree::get_resource_type(const String &p_path) const {
	return p_path.get_extension().to_lower() == "vbtree" ? "VBehaviourTreeResource" : "";
}

Error ResourceFormatSaverVBehaviourTree::save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags) {
	Ref<VBehaviourTreeResource> btree = p_resource;
	return btree->VSaveToFile(p_path);
}

bool ResourceFormatSaverVBehaviourTree::recognize(const Ref<Resource> &p_resource) const {
	return Object::cast_to<VBehaviourTreeResource>(*p_resource) != nullptr;
}

void ResourceFormatSaverVBehaviourTree::get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *r_extensions) const {
	r_extensions->push_back("vbtree");
}
} //namespace behaviour_tree
#endif
