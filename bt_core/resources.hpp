#pragma once

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "tree.hpp"

#include <vector>

namespace behaviour_tree {
class BehaviourTree;
class IBehaviourTreeNodeBehaviour;

class ResourceFormatLoaderBehaviourTree;
class ResourceFormatSaverBehaviourTree;

enum class NodeType {
	Action,
	Composite,
	Decorator
};

class BehaviourTreeResource : public Resource {
	GDCLASS(BehaviourTreeResource, Resource);

public:
	static void _bind_methods();
	static void register_types();
	static void unregister_types();

public:
	BehaviourTreeResource();
	Error LoadFromFile(const String &path, Ref<FileAccess> file = nullptr);
	Error SaveToFile(const String &path, Ref<FileAccess> file = nullptr);

	static inline Ref<ResourceFormatLoaderBehaviourTree> BTreeResLoader;
	static inline Ref<ResourceFormatSaverBehaviourTree> BTreeResSaver;

	Ref<BehaviourTree> GetTree() const noexcept {
		return m_Tree;
	}

	void SetTree(Ref<BehaviourTree> res) noexcept;

private:
	struct NodeLoadInfo {
		Ref<IBehaviourTreeNodeBehaviour> Node;
		NodeType Type;
		std::vector<uint16_t> Indices;
	};

	using NodeLoadInfoContainer = std::vector<NodeLoadInfo>;

	NodeLoadInfoContainer TreeToNodesLoadInfo();

	static NodeLoadInfoContainer LoadNodesFromFile(Ref<FileAccess> &file);
	static void SaveNodesToFile(const NodeLoadInfoContainer &loaded_nodes, IBehaviourTreeNodeBehaviour *root_node, Ref<FileAccess> &file);

	static void ResolveNodesChildrensFromFile(NodeLoadInfoContainer &nodes);
	static void ResolveNodesIndicesForFile(NodeLoadInfoContainer &nodes);

protected:
	static void WriteStringToFile(Ref<FileAccess> &file, const String &key);
	static String ReadStringFromFile(Ref<FileAccess> &file);

private:
	Ref<BehaviourTree> m_Tree;
};

class ResourceFormatLoaderBehaviourTree : public ResourceFormatLoader {
public:
	Ref<Resource> load(
			const String &p_path,
			const String &p_original_path = "",
			Error *r_error = nullptr,
			bool p_use_sub_threads = false,
			float *r_progress = nullptr,
			CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	void get_recognized_extensions(List<String> *r_extensions) const override;
	bool handles_type(const String &p_type) const override;
	String get_resource_type(const String &p_path) const override;
};

class ResourceFormatSaverBehaviourTree : public ResourceFormatSaver {
public:
	Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	bool recognize(const Ref<Resource> &p_resource) const override;
	void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *r_extensions) const override;
};
} //namespace behaviour_tree
