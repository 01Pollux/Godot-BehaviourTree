#pragma once

#include "../composite_node.hpp"

namespace behaviour_tree::nodes {
class BehaviourTreeFallbackNode : public IBehaviourTreeCompositeNode {
	GDCLASS(BehaviourTreeFallbackNode, IBehaviourTreeCompositeNode);

protected:
	NodeState OnExecute() override {
		for (size_t i = 0; i < m_Childrens.size(); i++) {
			if (m_Childrens[i]->GetState() != NodeState::Failure) {
				switch (m_Childrens[i]->Execute()) {
					case NodeState::Failure:
						continue;

					case NodeState::Success:
						return NodeState::Success;
				}
				return NodeState::Running;
			}
		}
		return NodeState::Failure;
	}
};
} //namespace behaviour_tree::nodes
