#pragma once

#include "../action_node.hpp"
#include "scene/main/node.h"

namespace behaviour_tree {
class BehaviourTree;
}
namespace behaviour_tree::nodes {
class BehaviourTreeCallFunctionNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeCallFunctionNode, IBehaviourTreeActionNode);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_target_node", "path"), &BehaviourTreeCallFunctionNode::SetTargetNode);
		ClassDB::bind_method(D_METHOD("get_target_node"), &BehaviourTreeCallFunctionNode::GetTargetNode);
		ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node"), "set_target_node", "get_target_node");

		ClassDB::bind_method(D_METHOD("set_callback_function", "name"), &BehaviourTreeCallFunctionNode::SetCallbackFunction);
		ClassDB::bind_method(D_METHOD("get_callback_function"), &BehaviourTreeCallFunctionNode::GetCallbackFunction);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "callback_function"), "set_callback_function", "get_callback_function");

		ClassDB::bind_method(D_METHOD("set_return_name", "name"), &BehaviourTreeCallFunctionNode::SetReturnName);
		ClassDB::bind_method(D_METHOD("get_return_name"), &BehaviourTreeCallFunctionNode::GetReturnName);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "return_name"), "set_return_name", "get_return_name");

		ClassDB::bind_method(D_METHOD("set_execute_deffered", "state"), &BehaviourTreeCallFunctionNode::SetDeffered);
		ClassDB::bind_method(D_METHOD("get_execute_deffered"), &BehaviourTreeCallFunctionNode::GetDeffered);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_deffered"), "set_execute_deffered", "get_execute_deffered");

		ClassDB::bind_method(D_METHOD("set_execute_rpc", "state"), &BehaviourTreeCallFunctionNode::SetRPC);
		ClassDB::bind_method(D_METHOD("get_execute_rpc"), &BehaviourTreeCallFunctionNode::GetRPC);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_rpc"), "set_execute_rpc", "get_execute_rpc");

		ClassDB::bind_method(D_METHOD("set_args", "args"), &BehaviourTreeCallFunctionNode::SetArgs);
		ClassDB::bind_method(D_METHOD("get_args"), &BehaviourTreeCallFunctionNode::GetArgs);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "arguments"), "set_args", "get_args");
	}

	void Initialize();

	void SerializeNode(Dictionary &out_data) const override {
		IBehaviourTreeActionNode::SerializeNode(out_data);

		out_data["path"] = m_TargetPath;
		out_data["args"] = m_Args;
		out_data["function"] = m_FunctionName;
		out_data["return"] = m_ReturnValueName;
		out_data["deffered"] = m_IsDeffered;
		out_data["rpc"] = m_IsRPC;
	}

	void DeserializeNode(const Dictionary &in_data) {
		SetTargetNode(in_data["path"]);
		m_Args = in_data["args"];
		m_FunctionName = in_data["function"];
		m_ReturnValueName = in_data["return"];
		m_IsDeffered = in_data["deffered"];
		m_IsRPC = in_data["rpc"];

		IBehaviourTreeActionNode::DeserializeNode(in_data);
	}

protected:
	NodeState OnExecute() override;

public:
	void SetTargetNode(const NodePath &target_node) {
		m_TargetPath = target_node;
	}
	NodePath GetTargetNode() const {
		return m_TargetPath;
	}

	void SetCallbackFunction(const String &signal) {
		m_FunctionName = signal;
	}
	String GetCallbackFunction() const {
		return m_FunctionName;
	}

	void SetReturnName(const String &name) {
		m_ReturnValueName = name;
	}
	String GetReturnName() const {
		return m_ReturnValueName;
	}

	void SetDeffered(bool state) {
		m_IsDeffered = state;
	}
	bool GetDeffered() const {
		return m_IsDeffered;
	}

	void SetRPC(bool state) {
		m_IsRPC = state;
	}
	bool GetRPC() const {
		return m_IsRPC;
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

	String m_FunctionName;
	String m_ReturnValueName;

	bool m_IsDeffered = false;
	bool m_IsRPC = false;
};
} //namespace behaviour_tree::nodes
