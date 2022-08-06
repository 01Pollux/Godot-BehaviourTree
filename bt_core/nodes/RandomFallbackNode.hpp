#pragma once

#include "FallbackNode.hpp"
#include <random>

namespace behaviour_tree::nodes {
class BehaviourTreeRandomFallbackNode : public BehaviourTreeFallbackNode {
	GDCLASS(BehaviourTreeRandomFallbackNode, IBehaviourTreeCompositeNode);

protected:
	void OnEnter() override {
		std::shuffle(m_Childrens.begin(), m_Childrens.end(), m_RandEngine);
	}

private:
	std::default_random_engine m_RandEngine{ std::random_device{}() };
};
} //namespace behaviour_tree::nodes
