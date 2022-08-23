#pragma once

#include "node_behaviour.hpp"

namespace behaviour_tree {
class IBehaviourTreeCompositeNode : public IBehaviourTreeNodeBehaviour {
	GDCLASS(IBehaviourTreeCompositeNode, IBehaviourTreeNodeBehaviour);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("add_btchild", "child"), &IBehaviourTreeCompositeNode::AddChild);
		ClassDB::bind_method(D_METHOD("remove_btchild", "child"), &IBehaviourTreeCompositeNode::RemoveChild);

		ClassDB::bind_method("get_btchildrens", &IBehaviourTreeCompositeNode::GDGetChildrens);
		ClassDB::bind_method("_set_btchildrens", &IBehaviourTreeCompositeNode::GDSetChildrens);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "bt_childrens", PROPERTY_HINT_ARRAY_TYPE, "", PROPERTY_USAGE_STORAGE), "_set_btchildrens", "get_btchildrens");
	}

public:
	bool GetChildrens(std::vector<IBehaviourTreeNodeBehaviour *> &childrens) const final {
		if (!m_Childrens.empty()) {
			childrens.reserve(childrens.size() + m_Childrens.size());
			for (auto &child : m_Childrens)
				childrens.emplace_back(const_cast<IBehaviourTreeNodeBehaviour *>(*child));
			return true;
		}
		return false;
	}

	auto &GetChildrens() noexcept {
		return m_Childrens;
	}

	void AddChild(Ref<IBehaviourTreeNodeBehaviour> child) {
		m_Childrens.emplace_back(child);
	}

	void RemoveChild(Ref<IBehaviourTreeNodeBehaviour> child) {
		auto iter = std::find(m_Childrens.begin(), m_Childrens.end(), child);
		if (iter != m_Childrens.end())
			m_Childrens.erase(iter);
	}

private:
	Array GDGetChildrens() {
		Array childrens;
		childrens.resize(m_Childrens.size());
		for (size_t i = 0; i < childrens.size(); i++)
			childrens[i] = m_Childrens[i];
		return childrens;
	}

	void GDSetChildrens(const Array &childrens) {
		m_Childrens.clear();
		m_Childrens.reserve(childrens.size());
		for (size_t i = 0; i < childrens.size(); i++)
			m_Childrens.emplace_back(childrens[i]);
	}

protected:
	std::vector<Ref<IBehaviourTreeNodeBehaviour>> m_Childrens;
};
} //namespace behaviour_tree
