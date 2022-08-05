#pragma once

#include "node_behaviour.hpp"

namespace behaviour_tree
{
	class IBehaviourTreeCompositeNode : public IBehaviourTreeNodeBehaviour
	{
		GDCLASS(IBehaviourTreeCompositeNode, IBehaviourTreeNodeBehaviour);

	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("add_child", "child"), &IBehaviourTreeCompositeNode::AddChild);
			ClassDB::bind_method(D_METHOD("remove_child", "child"), &IBehaviourTreeCompositeNode::RemoveChild);
		}

	public:
		bool GetChildrens(std::vector<IBehaviourTreeNodeBehaviour*>& childrens) final
		{
			if (!m_Childrens.empty())
			{
				childrens.reserve(childrens.size() + m_Childrens.size());
				for (auto& child : m_Childrens)
					childrens.emplace_back(child.ptr());
				return true;
			}
			return false;
		}

		auto& GetChildrens() noexcept
		{
			return m_Childrens;
		}

		void AddChild(Ref<IBehaviourTreeNodeBehaviour> child)
		{
			m_Childrens.emplace_back(child);
		}

		void RemoveChild(Ref<IBehaviourTreeNodeBehaviour> child)
		{
			auto iter = std::find(m_Childrens.begin(), m_Childrens.end(), child);
			if (iter != m_Childrens.end())
				m_Childrens.erase(iter);
		}

	protected:
		std::vector<Ref<IBehaviourTreeNodeBehaviour>> m_Childrens;
	};
}
