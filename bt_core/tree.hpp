#pragma once

#include "node_behaviour.hpp"
#include "scene/main/node.h"
#include "resources.hpp"

#include <functional>
#include <map>

namespace behaviour_tree {
class ResourceFormatLoaderBehaviourTree;
class ResourceFormatSaverBehaviourTree;

enum class NodeType {
	Action,
	Composite,
	Decorator
};

class BehaviourTree : public Resource {
	GDCLASS(BehaviourTree, Resource);

public:
	static void _bind_methods();
	static void register_types();
	static void unregister_types();

	BehaviourTree() {
		set_local_to_scene(true);
	}

	void reset_state() override;

	enum BehaviourTreeNodeState {
		BEHAVIOUR_TREE_NODE_INACTIVE = -1,
		BEHAVIOUR_TREE_NODE_RUNNING,
		BEHAVIOUR_TREE_NODE_SUCCESS,
		BEHAVIOUR_TREE_NODE_FAILURE
	};

	Error LoadFromFile(const String &path, Ref<FileAccess> file = nullptr);
	Error SaveToFile(const String &path, Ref<FileAccess> file = nullptr);

	static inline Ref<ResourceFormatLoaderBehaviourTree> BTreeResLoader;
	static inline Ref<ResourceFormatSaverBehaviourTree> BTreeResSaver;

public:
	static void Traverse(IBehaviourTreeNodeBehaviour *node, const std::function<void(IBehaviourTreeNodeBehaviour *)> &callback);

	bool IsAlwaysRunning() const noexcept {
		return m_RunAlways;
	}

	void SetAlwaysRunning(bool value) {
		m_RunAlways = value;
	}

	void Rewind() {
		if (m_RootNodesIndex == -1) {
			for (auto &node : m_Nodes) {
				node->Rewind();
			}
		} else {
			Traverse(m_Nodes[m_RootNodesIndex].ptr(), [](IBehaviourTreeNodeBehaviour *cur_node) {
				cur_node->Rewind();
			});
		}
	}

	void InitializeTree() {
		for (auto &node : m_Nodes) {
			node->SetBehaviourTree(Ref(this));
			node->Initialize();
		}
	}
	void ExecuteTree();

	void SetRootNode(Ref<IBehaviourTreeNodeBehaviour> node) {
		for (size_t i = 0; i < m_Nodes.size(); i++) {
			if (m_Nodes[i] == node) {
				m_RootNodesIndex = i;
				return;
			}
		}
		m_RootNodesIndex = -1;
	}

	Ref<IBehaviourTreeNodeBehaviour> GDCreateNode(const String &node_name) {
		if (ClassDB::is_parent_class(node_name, "IBehaviourTreeNodeBehaviour")) {
			Object *node_obj = ClassDB::instantiate(node_name);
			if (node_obj) {
				Ref node = Object::cast_to<IBehaviourTreeNodeBehaviour>(node_obj);
				m_Nodes.push_back(node);
				return node;
			}
		}
		return nullptr;
	}

	void GDRemoveNode(Ref<IBehaviourTreeNodeBehaviour> node) {
		for (auto iter = m_Nodes.begin(); iter != m_Nodes.end(); iter++) {
			if (*iter == node) {
				if (*iter == GetRootNode())
					m_RootNodesIndex = -1;

				DisconnectConnectedNodes(*node);
				m_Nodes.erase(iter);
				break;
			}
		}
	}

	void GDRemoveNodeByIndex(int index) {
		if (m_Nodes.size() > index) {
			if (m_Nodes[index] == GetRootNode())
				m_RootNodesIndex = -1;

			DisconnectConnectedNodes(*m_Nodes[index]);
			m_Nodes.erase(m_Nodes.begin() + index);
		}
	}

	Ref<IBehaviourTreeNodeBehaviour> GetParentOfNode(IBehaviourTreeNodeBehaviour *node);

	auto &GetNodes() noexcept {
		return m_Nodes;
	}

	Ref<IBehaviourTreeNodeBehaviour> GDGetRootNode() {
		return m_RootNodesIndex != -1 ? m_Nodes[m_RootNodesIndex] : nullptr;
	}

	IBehaviourTreeNodeBehaviour *GetRootNode() {
		return m_RootNodesIndex != -1 ? *m_Nodes[m_RootNodesIndex] : nullptr;
	}

	Ref<Resource> duplicate(bool) const override;
	void setup_local_to_scene() override;

	void SetBlackboard(const String& key, const Variant& value) {
		m_Blackboard[key] = value;
	}
	Variant GetBlackboard(const String& key) const {
		auto iter = m_Blackboard.find(key);
		return iter != m_Blackboard.end() ? iter->second : Variant{};
	}

private:
	void DisconnectConnectedNodes(IBehaviourTreeNodeBehaviour *node);

private:
	void GDSetRootNodeIndex(int index) {
		m_RootNodesIndex = index;
	}

	int GDGetRootNodeIndex() const {
		return m_RootNodesIndex;
	}

	void GDSetNodes(const Array &nodes) {
		m_Nodes.clear();
		m_Nodes.reserve(nodes.size());
		for (int i = 0; i < nodes.size(); i++)
			m_Nodes.emplace_back(nodes[i]);
	}

	Array GDGetNodes() const {
		Array nodes;
		nodes.resize(m_Nodes.size());
		for (int i = 0; i < nodes.size(); i++)
			nodes[i] = m_Nodes[i];
		return nodes;
	}

private:
	std::vector<Ref<IBehaviourTreeNodeBehaviour>> m_Nodes;
	std::map<String, Variant> m_Blackboard;

	int m_RootNodesIndex = -1;
	bool m_RunAlways = true;
	bool m_IsUnique = false;
};

class BehaviourTreeHolder : public Node {
	GDCLASS(BehaviourTreeHolder, Node);

public:
	static void _bind_methods();

	void _notification(int p_notification);

private:
	void SetTargetPath(const NodePath &target_path) {
		if (m_TargetPath != target_path) {
			m_TargetPath = target_path;
		}
	}

	NodePath GetTargetPath() {
		return m_TargetPath;
	}

	void SetBehaviourTreeRes(const Ref<BehaviourTree> &tree) {
		m_Tree = tree;
	}
	Ref<BehaviourTree> GetBehaviourTree() {
		return m_Tree;
	}

	void ExecuteTree() {
		m_Tree->ExecuteTree();
	}

private:
	NodePath m_TargetPath;
	Ref<BehaviourTree> m_Tree;
};
} //namespace behaviour_tree
VARIANT_ENUM_CAST(behaviour_tree::BehaviourTree::BehaviourTreeNodeState);
