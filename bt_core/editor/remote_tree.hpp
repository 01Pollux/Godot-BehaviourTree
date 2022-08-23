#pragma once

#if TOOLS_ENABLED

#include "../visual_resources.hpp"
#include "scene/main/node.h"

namespace behaviour_tree {
class BehaviourTreeRemoteTreeHolder : public Node {
	GDCLASS(BehaviourTreeRemoteTreeHolder, Node);

public:
	static void _bind_methods() {

		ClassDB::bind_method(D_METHOD("set_vbehaviour_tree", "tree"), &BehaviourTreeRemoteTreeHolder::SetVisualTree);
		ClassDB::bind_method(D_METHOD("get_vbehaviour_tree"), &BehaviourTreeRemoteTreeHolder::GetVisualTree);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "vbehaviour_tree", PROPERTY_HINT_RESOURCE_TYPE, "VBehaviourTreeResource", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NO_EDITOR), "set_vbehaviour_tree", "get_vbehaviour_tree");

		ClassDB::bind_method(D_METHOD("set_remote_states"), &BehaviourTreeRemoteTreeHolder::SetRemoteStates);
		ClassDB::bind_method(D_METHOD("get_remote_states"), &BehaviourTreeRemoteTreeHolder::GetRemoteStates);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "remote_states", PROPERTY_HINT_ARRAY_TYPE, "int", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NO_EDITOR), "set_remote_states", "get_remote_states");
	}

	void SetVisualTree(Ref<VisualBehaviourTree> vtree) {
		if (m_Tree.is_valid())
			m_Tree->disconnect("_on_btree_execute", callable_mp(this, &BehaviourTreeRemoteTreeHolder::SyncStates));

		m_Tree = vtree;
		m_Tree->connect("_on_btree_execute", callable_mp(this, &BehaviourTreeRemoteTreeHolder::SyncStates));
	}

	Ref<VisualBehaviourTree> GetVisualTree() const {
		return m_Tree;
	}

	void SyncStates() {
		m_RemoteStates.clear();
		auto& nodes = m_Tree->GetNodes();
		m_RemoteStates.resize(nodes.size());
		for (int i = 0; i < m_RemoteStates.size(); i++) {
			m_RemoteStates[i] = nodes[i]->GetState<int>();
		}
	}

	void SetRemoteStates(const Array& states) {
		m_RemoteStates = states;
	}
	
	Array GetRemoteStates() const {
		return m_RemoteStates;
	}

private:
	Ref<VisualBehaviourTree> m_Tree;
	Array m_RemoteStates;
};
} //namespace behaviour_tree

#endif
