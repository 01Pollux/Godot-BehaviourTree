#pragma once

#include "../decorator_node.hpp"

namespace behaviour_tree::nodes {
class BehaviourTreeAlwaysFailureNode : public IBehaviourTreeDecoratorNode {
	GDCLASS(BehaviourTreeAlwaysFailureNode, IBehaviourTreeDecoratorNode);

protected:
	NodeState OnExecute() override {
		return m_Child->Execute() != NodeState::Running ? NodeState::Failure : NodeState::Running;
	}
};
} //namespace behaviour_tree::nodes
