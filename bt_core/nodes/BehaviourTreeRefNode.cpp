
#include "BehaviourTreeRefNode.hpp"
#include "../tree.hpp"

namespace behaviour_tree::nodes
{
	void BehaviourTreeRefNode::Rewind()
	{
		IBehaviourTreeActionNode::Rewind();
		m_Tree->GetTree()->Rewind();
	}

	void BehaviourTreeRefNode::OnEnter()
	{
		m_Tree->GetTree()->Rewind();
	}

	NodeState BehaviourTreeRefNode::OnExecute()
	{
		m_Tree->GetTree()->ExecuteTree();
		return m_Tree->GetTree()->GetRootNode()->GetState();
	}
}
