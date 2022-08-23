
#include "node_behaviour.hpp"
#include "tree.hpp"

namespace behaviour_tree {
void IBehaviourTreeNodeBehaviour::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_btnode_state"), &IBehaviourTreeNodeBehaviour::SetState<int>);
	ClassDB::bind_method(D_METHOD("get_btnode_state"), &IBehaviourTreeNodeBehaviour::GetState<int>);
	ClassDB::bind_method(D_METHOD("do_abort"), &IBehaviourTreeNodeBehaviour::Abort);

	ClassDB::bind_method(D_METHOD("_set_bt_data", "data"), &IBehaviourTreeNodeBehaviour::SetData);
	ClassDB::bind_method(D_METHOD("_get_bt_data"), &IBehaviourTreeNodeBehaviour::GetData);
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "bt_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_bt_data", "_get_bt_data");

	ClassDB::bind_method(D_METHOD("get_behaviour_tree"), &IBehaviourTreeNodeBehaviour::GetBehaviourTree);
}

void IBehaviourTreeNodeBehaviour::Abort() {
	BehaviourTree::Traverse(
			this,
			[](IBehaviourTreeNodeBehaviour *node) {
				if (node->GetState() != NodeState::Inactive)
					node->OnExit();
				node->Rewind();
			});
}

Ref<BehaviourTree> IBehaviourTreeNodeBehaviour::GetBehaviourTree() const {
	return m_Tree;
}

void IBehaviourTreeNodeBehaviour::SetBehaviourTree(Ref<BehaviourTree> tree) {
	m_Tree = tree;
}

void IBehaviourTreeNodeBehaviour::SetData(const Dictionary &data) {
	DeserializeNode(data);
}
Dictionary IBehaviourTreeNodeBehaviour::GetData() const {
	Dictionary data;
	SerializeNode(data);
	return data;
}
} //namespace behaviour_tree
