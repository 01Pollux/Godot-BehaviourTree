#pragma once

#include "../action_node.hpp"

namespace behaviour_tree {
class BehaviourTree;
}
namespace behaviour_tree::nodes {
class BehaviourTreeCallFunctionNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeCallFunctionNode, IBehaviourTreeActionNode);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_callback_function", "name"), &BehaviourTreeCallFunctionNode::SetCallbackFunction);
		ClassDB::bind_method(D_METHOD("get_callback_function"), &BehaviourTreeCallFunctionNode::GetCallbackFunction);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "callback_function"), "set_callback_function", "get_callback_function");

		ClassDB::bind_method(D_METHOD("set_execute_deffered", "state"), &BehaviourTreeCallFunctionNode::SetDeffered);
		ClassDB::bind_method(D_METHOD("get_execute_deffered"), &BehaviourTreeCallFunctionNode::GetDeffered);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_deffered"), "set_execute_deffered", "get_execute_deffered");

		ClassDB::bind_method(D_METHOD("set_args", "args"), &BehaviourTreeCallFunctionNode::SetArgs);
		ClassDB::bind_method(D_METHOD("get_args"), &BehaviourTreeCallFunctionNode::GetArgs);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "arguments"), "set_args", "get_args");
	}

	void Initialize() override;

	void SerializeNode(Dictionary &out_data) const override {
		IBehaviourTreeActionNode::SerializeNode(out_data);

		out_data["args"] = m_Args;
		out_data["function"] = m_FunctionName;
		out_data["deffered"] = m_IsDeffered;
	}

	void DeserializeNode(const Dictionary &in_data) {
		m_Args.clear();
		m_Args = in_data["args"];
		m_FunctionName = in_data["function"];
		m_IsDeffered = in_data["deffered"];

		IBehaviourTreeActionNode::DeserializeNode(in_data);
	}

protected:
	NodeState OnExecute() override;

private:
	void SetCallbackFunction(const String &signal) {
		m_FunctionName = signal;
	}

	String GetCallbackFunction() const {
		return m_FunctionName;
	}

	void SetDeffered(bool state) {
		m_IsDeffered = state;
	}

	bool GetDeffered() const {
		return m_IsDeffered;
	}

	void SetArgs(const Vector<Variant> &args) {
		m_Args = args;
	}

	Vector<Variant> GetArgs() const {
		return m_Args;
	}

private:
	Ref<BehaviourTree> m_Tree;
	Object *m_TargetObject = nullptr;

	String m_FunctionName;
	Vector<Variant> m_Args;
	std::vector<const Variant *> m_TmpArgs;

	bool m_IsDeffered = false;
};
} //namespace behaviour_tree::nodes
