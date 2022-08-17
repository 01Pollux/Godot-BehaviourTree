#pragma once

#include "resources.hpp"
#include "tree.hpp"

#include <vector>

#if TOOLS_ENABLED
namespace behaviour_tree {
class ResourceFormatLoaderVBehaviourTree;
class ResourceFormatSaverVBehaviourTree;

class VBehaviourTreeResource : public BehaviourTreeResource {
	GDCLASS(VBehaviourTreeResource, BehaviourTreeResource);

public:
	static void _bind_methods();

	struct VisualNodeInfo {
		Vector2 Position;
		String Title;
		String Comment;
	};

	static void register_types();
	static void unregister_types();

public:
	Error VLoadFromFile(const String &path);
	Error VSaveToFile(const String &path);

	static inline Ref<ResourceFormatLoaderVBehaviourTree> VBTreeResLoader;
	static inline Ref<ResourceFormatSaverVBehaviourTree> VBTreeResSaver;

	VisualNodeInfo &GetNodeInfo(size_t index) noexcept {
		return m_NodesInfo[index];
	}

	auto &GetNodesInfo() noexcept {
		return m_NodesInfo;
	}

	Dictionary GetCustomNodeInfo(const String &name) {
		return m_CustomNodesDescriptor.has(name) ? m_CustomNodesDescriptor[name] : Dictionary{};
	}

	VisualNodeInfo &AddNodeInfo() noexcept {
		return m_NodesInfo.emplace_back();
	}

	void RemoveNodeInfo(int node_index) noexcept {
		m_NodesInfo.erase(m_NodesInfo.begin() + node_index);
	}

private:
	void SetNodesDataPath(const String &file_path);
	String GetNodesDataPath() const;

private:
	std::vector<VisualNodeInfo> m_NodesInfo;
	Dictionary m_CustomNodesDescriptor;
};

class ResourceFormatLoaderVBehaviourTree : public ResourceFormatLoader {
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

class ResourceFormatSaverVBehaviourTree : public ResourceFormatSaver {
public:
	Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	bool recognize(const Ref<Resource> &p_resource) const override;
	void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *r_extensions) const override;
};
} //namespace behaviour_tree
#endif
