#pragma once

#include "../decorator_node.hpp"

namespace behaviour_tree::nodes {
class BehaviourTreeConverterNode : public IBehaviourTreeDecoratorNode {
	GDCLASS(BehaviourTreeConverterNode, IBehaviourTreeDecoratorNode);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_success", "node_state"), &BehaviourTreeConverterNode::SetStateI<NodeState::Success>);
		ClassDB::bind_method(D_METHOD("get_success"), &BehaviourTreeConverterNode::GetStateI<NodeState::Success>);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "success_state", PROPERTY_HINT_ENUM, "running,success,failure"), "set_success", "get_success");

		ClassDB::bind_method(D_METHOD("set_running", "node_state"), &BehaviourTreeConverterNode::SetStateI<NodeState::Running>);
		ClassDB::bind_method(D_METHOD("get_running"), &BehaviourTreeConverterNode::GetStateI<NodeState::Running>);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "running_state", PROPERTY_HINT_ENUM, "running,success,failure"), "set_running", "get_running");

		ClassDB::bind_method(D_METHOD("set_failure", "node_state"), &BehaviourTreeConverterNode::SetStateI<NodeState::Failure>);
		ClassDB::bind_method(D_METHOD("get_failure"), &BehaviourTreeConverterNode::GetStateI<NodeState::Failure>);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "failure_state", PROPERTY_HINT_ENUM, "running,success,failure"), "set_failure", "get_failure");
	}

	void SerializeNode(Dictionary &out_data) const override {
		IBehaviourTreeDecoratorNode::SerializeNode(out_data);
		out_data["success"] = static_cast<int>(m_Success);
		out_data["failure"] = static_cast<int>(m_Failure);
		out_data["running"] = static_cast<int>(m_Running);
	}

	void DeserializeNode(const Dictionary &in_data) {
		m_Success = static_cast<NodeState>(static_cast<int>(in_data["success"]));
		m_Failure = static_cast<NodeState>(static_cast<int>(in_data["failure"]));
		m_Running = static_cast<NodeState>(static_cast<int>(in_data["running"]));
		IBehaviourTreeDecoratorNode::DeserializeNode(in_data);
	}

protected:
	NodeState OnExecute() final {
		switch (m_Child->Execute()) {
			case NodeState::Success:
				return m_Success;
			case NodeState::Failure:
				return m_Failure;
			default:
				return m_Running;
		}
	}

public:
	template <NodeState _From>
	void SetState(NodeState to) {
		if constexpr (_From == NodeState::Success)
			m_Success = to;
		else if constexpr (_From == NodeState::Failure)
			m_Failure = to;
		else if constexpr (_From == NodeState::Running)
			m_Running = to;
	}

	template <NodeState _From>
	NodeState GetState() {
		if constexpr (_From == NodeState::Success)
			return m_Success;
		else if constexpr (_From == NodeState::Failure)
			return m_Failure;
		else if constexpr (_From == NodeState::Running)
			return m_Running;
		else
			return NodeState::Inactive;
	}

private:
	template <NodeState _From>
	void SetStateI(int to) {
		SetState<_From>(static_cast<NodeState>(to));
	}

	template <NodeState _From>
	int GetStateI() {
		return static_cast<int>(GetState<_From>());
	}

private:
	NodeState
			m_Success = NodeState::Success,
			m_Failure = NodeState::Failure,
			m_Running = NodeState::Running;
};
} //namespace behaviour_tree::nodes
