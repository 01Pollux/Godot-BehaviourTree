
#pragma once

#include "../decorator_node.hpp"

namespace behaviour_tree::nodes
{
	class BehaviourTreeAlwaysSuccessNode : public IBehaviourTreeDecoratorNode
	{
		GDCLASS(BehaviourTreeAlwaysSuccessNode, IBehaviourTreeDecoratorNode);
	protected:
		NodeState OnExecute() override
		{
			return m_Child->Execute() != NodeState::Running ? NodeState::Success : NodeState::Running;
		}
	};
}
