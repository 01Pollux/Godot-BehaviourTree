#pragma once

#include "../action_node.hpp"
#include "core/debugger/engine_debugger.h"

namespace behaviour_tree {
class BehaviourTree;
}
namespace behaviour_tree::nodes {
class BehaviourTreeBreakPointNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeBreakPointNode, IBehaviourTreeActionNode);

protected:
	NodeState OnExecute() override {
		EngineDebugger::get_singleton()->debug();
		return NodeState::Success;
	}
};
} //namespace behaviour_tree::nodes
