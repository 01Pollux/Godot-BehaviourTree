#pragma once

#include "../action_node.hpp"

namespace behaviour_tree {
class BehaviourTree;
}
namespace behaviour_tree::nodes {
class BehaviourTreeEmitSignalNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeEmitSignalNode, IBehaviourTreeActionNode);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_target_node", "path"), &BehaviourTreeEmitSignalNode::SetTargetNode);
		ClassDB::bind_method(D_METHOD("get_target_node"), &BehaviourTreeEmitSignalNode::GetTargetNode);
		ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node"), "set_target_node", "get_target_node");

		ClassDB::bind_method(D_METHOD("set_signal_name", "name"), &BehaviourTreeEmitSignalNode::SetCallbackSignal);
		ClassDB::bind_method(D_METHOD("get_signal_name"), &BehaviourTreeEmitSignalNode::GetCallbackSignal);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "signal_name"), "set_signal_name", "get_signal_name");

		ClassDB::bind_method(D_METHOD("set_args", "args"), &BehaviourTreeEmitSignalNode::SetArgs);
		ClassDB::bind_method(D_METHOD("get_args"), &BehaviourTreeEmitSignalNode::GetArgs);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "arguments"), "set_args", "get_args");

#if TOOLS_ENABLED
		ADD_SIGNAL(MethodInfo("_btree_signal_node_path_changed"));
#endif
	}

	void Initialize() override;

	void SerializeNode(Dictionary &out_data) const override {
		IBehaviourTreeActionNode::SerializeNode(out_data);

		out_data["path"] = m_TargetPath;
		out_data["args"] = m_Args;
		out_data["singal"] = m_SignalName;
	}

	void DeserializeNode(const Dictionary &in_data) {
		SetTargetNode(in_data["path"]);
		m_Args = in_data["args"];
		m_SignalName = in_data["siganl"];

		IBehaviourTreeActionNode::DeserializeNode(in_data);
	}

protected:
	NodeState OnExecute() override;

public:
	void SetTargetNode(NodePath target_node) {
		m_TargetPath = target_node;
#if TOOLS_ENABLED
		emit_signal("_btree_signal_node_path_changed");
#endif
	}
	NodePath GetTargetNode() const {
		return m_TargetPath;
	}

	void SetCallbackSignal(const String &signal) {
		m_SignalName = signal;
	}
	String GetCallbackSignal() const {
		return m_SignalName;
	}

	void SetArgs(const Array &args) {
		m_Args = args;
	}
	Array GetArgs() const {
		return m_Args;
	}

private:
	Node *m_TargetNode = nullptr;
	NodePath m_TargetPath;

	Array m_Args;
	String m_SignalName;
};
} //namespace behaviour_tree::nodes
