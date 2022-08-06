#pragma once

#include "FallbackNode.hpp"
#include <random>

namespace behaviour_tree::nodes {
class BehaviourTreeInterruptorNode : public IBehaviourTreeCompositeNode {
	GDCLASS(BehaviourTreeInterruptorNode, IBehaviourTreeCompositeNode);

protected:
	NodeState OnExecute() override {
		for (size_t i = 0; i < m_Childrens.size(); i++) {
			NodeState state = m_Childrens[i]->Execute();
			if (state != NodeState::Failure) {
				for (size_t j = 0; j < m_Childrens.size(); j++)
					m_Childrens[j]->Abort();
				return state;
			}
		}
		return NodeState::Failure;
	}
};
} //namespace behaviour_tree::nodes
