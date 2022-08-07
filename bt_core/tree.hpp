#pragma once

#include "node_behaviour.hpp"
#include "scene/main/node.h"
#include <functional>
#include <map>

namespace behaviour_tree {
class BehaviourTree : public RefCounted {
	GDCLASS(BehaviourTree, RefCounted);

public:
	static void _bind_methods();
	static void register_types();

	enum BehaviourTreeNodeState {
		BEHAVIOUR_TREE_NODE_INACTIVE = -1,
		BEHAVIOUR_TREE_NODE_RUNNING,
		BEHAVIOUR_TREE_NODE_SUCCESS,
		BEHAVIOUR_TREE_NODE_FAILURE
	};

public:
	static void Traverse(IBehaviourTreeNodeBehaviour *node, const std::function<void(IBehaviourTreeNodeBehaviour *)> &callback);

	bool IsAlwaysRunning() const noexcept {
		return m_RunAlways;
	}

	void SetAlwaysRunning(bool value) {
		m_RunAlways = value;
	}

	void Rewind() {
		Traverse(m_RootNode.ptr(), [](IBehaviourTreeNodeBehaviour *cur_node) {
			cur_node->Rewind();
		});
	}

	void ExecuteTree();

	void SetRootNode(Ref<IBehaviourTreeNodeBehaviour> node) {
		m_RootNode = node;
		node.unref();
	}

	Ref<IBehaviourTreeNodeBehaviour> GDCreateNode(const String &node_name) {
		if (ClassDB::is_parent_class(node_name, "IBehaviourTreeNodeBehaviour")) {
			Object *node_obj = ClassDB::instantiate(node_name);
			if (node_obj) {
				IBehaviourTreeNodeBehaviour *node = Object::cast_to<IBehaviourTreeNodeBehaviour>(node_obj);
				m_Nodes.push_back(Ref(node));
				return node;
			}
		}
		return nullptr;
	}

	void GDRemoveNode(Ref<IBehaviourTreeNodeBehaviour> node) {
		for (auto iter = m_Nodes.begin(); iter != m_Nodes.end(); iter++) {
			if (*iter == node) {
				if (*iter == *m_RootNode)
					m_RootNode = nullptr;
				DisconnectConnectedNodes(*node);
				m_Nodes.erase(iter);
				break;
			}
		}
	}

	void GDRemoveNodeByIndex(int index) {
		if (m_Nodes.size() > index) {
			if (m_Nodes[index] == *m_RootNode)
				m_RootNode = nullptr;
			DisconnectConnectedNodes(*m_Nodes[index]);
			m_Nodes.erase(m_Nodes.begin() + index);
		}
	}

	Ref<IBehaviourTreeNodeBehaviour> GetParentOfNode(IBehaviourTreeNodeBehaviour *node);

	auto &GetNodes() noexcept {
		return m_Nodes;
	}

	Ref<IBehaviourTreeNodeBehaviour> GDGetRootNode() {
		return m_RootNode.ptr();
	}

	IBehaviourTreeNodeBehaviour *GetRootNode() {
		return m_RootNode.ptr();
	}

private:
	void Initialize();

	void DisconnectConnectedNodes(IBehaviourTreeNodeBehaviour *node);

private:
	bool m_RunAlways = true;
	Ref<IBehaviourTreeNodeBehaviour> m_RootNode;
	std::vector<Ref<IBehaviourTreeNodeBehaviour>> m_Nodes;
	std::map<String, Variant> m_Blackboard;
};
} //namespace behaviour_tree
VARIANT_ENUM_CAST(behaviour_tree::BehaviourTree::BehaviourTreeNodeState);
