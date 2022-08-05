#pragma once

#include "../action_node.hpp"

namespace behaviour_tree::nodes
{
	class BehaviourTreePrintMessageNode : public IBehaviourTreeActionNode
	{
		GDCLASS(BehaviourTreePrintMessageNode, IBehaviourTreeActionNode);
	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("_set_message", "message"), &BehaviourTreePrintMessageNode::SetMessage);
			ClassDB::bind_method(D_METHOD("_get_message"), &BehaviourTreePrintMessageNode::GetMessage);

			ADD_PROPERTY(PropertyInfo(Variant::STRING, "message"), "_set_message", "_get_message");
		}

		void SerializeNode(Dictionary& out_data) const override
		{
			IBehaviourTreeActionNode::SerializeNode(out_data);
			out_data["message"] = m_Message;
		}

		void DeserializeNode(const Dictionary& in_data)
		{
			m_Message = in_data["message"];
			IBehaviourTreeActionNode::DeserializeNode(in_data);
		}

	protected:
		NodeState OnExecute() override
		{
			print_line(m_Message);
			return NodeState::Success;
		}

	private:
		String GetMessage()
		{
			return m_Message;
		}

		void SetMessage(const String& message)
		{
			m_Message = message;
		}

	private:
		StringName m_Message = "";
	};
}
