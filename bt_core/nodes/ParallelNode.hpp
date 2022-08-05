#pragma once

#include "../composite_node.hpp"

namespace behaviour_tree::nodes
{
	class BehaviourTreeParallelNode : public IBehaviourTreeCompositeNode
	{
		GDCLASS(BehaviourTreeParallelNode, IBehaviourTreeCompositeNode);
	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("set_focus_index"), &BehaviourTreeParallelNode::SetFocusChildIndex);
			ClassDB::bind_method(D_METHOD("get_focus_index"), &BehaviourTreeParallelNode::GetFocusChildIndex);
			ADD_PROPERTY(PropertyInfo(Variant::INT, "focus_index"), "set_focus_index", "get_focus_index");
		}

		void SerializeNode(Dictionary& out_data) const override
		{
			IBehaviourTreeCompositeNode::SerializeNode(out_data);
			out_data["focus_index"] = m_FocusChildIndex;
		}

		void DeserializeNode(const Dictionary& in_data)
		{
			m_FocusChildIndex = in_data["focus_index"];
			IBehaviourTreeCompositeNode::DeserializeNode(in_data);
		}

	protected:
		NodeState OnExecute() override
		{
			for (auto& child : m_Childrens)
				child->Execute();

			return m_Childrens[m_FocusChildIndex]->GetState();
		}

	private:
		void SetFocusChildIndex(int index)
		{
			m_FocusChildIndex = index;
		}

		int GetFocusChildIndex()
		{
			return m_FocusChildIndex;
		}

		int m_FocusChildIndex = -1;
	};
}
