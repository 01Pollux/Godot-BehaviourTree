#pragma once

#include "../action_node.hpp"
#include "core/os/os.h"

namespace behaviour_tree::nodes {
class BehaviourTreeWaitTimeNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeWaitTimeNode, IBehaviourTreeActionNode);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_duration", "duration"), &BehaviourTreeWaitTimeNode::SetWaitDuration);
		ClassDB::bind_method(D_METHOD("get_duration"), &BehaviourTreeWaitTimeNode::GetWaitDuration);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "set_duration", "get_duration");
	}

	void SerializeNode(Dictionary &out_data) const override {
		IBehaviourTreeActionNode::SerializeNode(out_data);
		out_data["duration"] = m_Duration;
	}

	void DeserializeNode(const Dictionary &in_data) {
		m_Duration = in_data["duration"];
		IBehaviourTreeActionNode::DeserializeNode(in_data);
	}

protected:
	void OnEnter() override {
		m_CurTime = OS::get_singleton()->get_unix_time() + m_Duration;
	}

	NodeState OnExecute() override {
		return m_CurTime < OS::get_singleton()->get_unix_time() ? NodeState::Success : NodeState::Running;
	}

private:
	void SetWaitDuration(double duration) {
		m_Duration = duration;
	}
	double GetWaitDuration() const {
		return m_Duration;
	}

private:
	double m_Duration = 0.f;
	double m_CurTime = 0.f;
};
} //namespace behaviour_tree::nodes
