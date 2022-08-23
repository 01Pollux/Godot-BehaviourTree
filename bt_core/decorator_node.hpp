#pragma once

#include "node_behaviour.hpp"

namespace behaviour_tree {
class IBehaviourTreeDecoratorNode : public IBehaviourTreeNodeBehaviour {
	GDCLASS(IBehaviourTreeDecoratorNode, IBehaviourTreeNodeBehaviour);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_btchild", "child"), &IBehaviourTreeDecoratorNode::SetChild);
		ClassDB::bind_method(D_METHOD("get_btchild"), &IBehaviourTreeDecoratorNode::GetChild);
	}

public:
	bool GetChildrens(std::vector<IBehaviourTreeNodeBehaviour *> &childrens) const final {
		if (m_Child.is_valid()) {
			childrens.emplace_back(const_cast<IBehaviourTreeNodeBehaviour*>(*m_Child));
			return true;
		}
		return false;
	}

	void SetChild(Ref<IBehaviourTreeNodeBehaviour> node) noexcept {
		m_Child = node;
	}

	Ref<IBehaviourTreeNodeBehaviour> GetChild() const noexcept {
		return m_Child;
	}

protected:
	Ref<IBehaviourTreeNodeBehaviour> m_Child;
};
} //namespace behaviour_tree
