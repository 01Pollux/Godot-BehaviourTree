#pragma once

#include "../decorator_node.hpp"
#include "core/os/os.h"

namespace behaviour_tree::nodes
{
	class BehaviourTreeTimeOutNode : public IBehaviourTreeDecoratorNode
	{
		GDCLASS(BehaviourTreeTimeOutNode, IBehaviourTreeDecoratorNode);
	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("set_timeout", "duration"), &BehaviourTreeTimeOutNode::SetTimeOut);
			ClassDB::bind_method(D_METHOD("get_timeout"), &BehaviourTreeTimeOutNode::GetTimeOut);
			ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "timeout"), "set_timeout", "get_timeout");
		}

		void SerializeNode(Dictionary& out_data) const override
		{
			out_data["duration"] = m_Duration;
			IBehaviourTreeDecoratorNode::SerializeNode(out_data);
		}

		void DeserializeNode(const Dictionary& in_data)
		{
			IBehaviourTreeDecoratorNode::DeserializeNode(in_data);
			m_Duration = in_data["duration"];
		}

	protected:
		void OnEnter() override
		{
			m_CurTime = OS::get_singleton()->get_unix_time() + m_Duration;
		}

		NodeState OnExecute() override
		{
			if (m_CurTime < OS::get_singleton()->get_unix_time())
				return NodeState::Failure;
			return m_Child->Execute();
		}

	private:
		void SetTimeOut(double duration)
		{
			m_Duration = duration;
		}
		double GetTimeOut() const
		{
			return m_Duration;
		}

	private:
		double m_Duration = 0.f;
		double m_CurTime = 0.f;
	};
}
