
#include "tree.hpp"
#include "node_behaviour.hpp"

namespace behaviour_tree
{
	void IBehaviourTreeNodeBehaviour::_bind_methods()
	{
		ClassDB::bind_method(D_METHOD("set_btnode_state"), &IBehaviourTreeNodeBehaviour::SetState<int>);
		ClassDB::bind_method(D_METHOD("get_btnode_state"), &IBehaviourTreeNodeBehaviour::GetState<int>);

	}

	void IBehaviourTreeNodeBehaviour::Abort()
	{
		BehaviourTree::Traverse(
			this,
			[](IBehaviourTreeNodeBehaviour* node)
			{
				if (node->GetState() != NodeState::Inactive)
					node->OnExit();
				node->Rewind();
			}
		);
	}
}
