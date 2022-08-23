
#include "BehaviourTreeRefNode.hpp"
#include "../tree.hpp"

namespace behaviour_tree::nodes {
void BehaviourTreeRefNode::Rewind() {
	IBehaviourTreeActionNode::Rewind();
	m_Tree->Rewind();
}

void BehaviourTreeRefNode::OnEnter() {
	m_Tree->Rewind();
}

NodeState BehaviourTreeRefNode::OnExecute() {
	m_Tree->ExecuteTree();
	return m_Tree->GetRootNode()->GetState();
}
} //namespace behaviour_tree::nodes
