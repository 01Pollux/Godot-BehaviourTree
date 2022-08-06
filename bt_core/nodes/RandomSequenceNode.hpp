#pragma once

#include "SequenceNode.hpp"
#include <random>

namespace behaviour_tree::nodes {
class BehaviourTreeRandomSequenceNode : public BehaviourTreeSequenceNode {
	GDCLASS(BehaviourTreeRandomSequenceNode, IBehaviourTreeCompositeNode);

protected:
	void OnEnter() override {
		std::shuffle(m_Childrens.begin(), m_Childrens.end(), m_RandEngine);
	}

private:
	std::default_random_engine m_RandEngine{ std::random_device{}() };
};
} //namespace behaviour_tree::nodes
