
#include "CallFunctionNode.hpp"
#include "../tree.hpp"

namespace behaviour_tree::nodes {
void BehaviourTreeCallFunctionNode::Initialize() {
	Node *holder = Object::cast_to<Node>(GetBehaviourTree()->GetBlackboard("bt_target_node"));
	if (holder && !m_TargetPath.is_empty())
		m_TargetNode = holder->get_node(m_TargetPath);
	else
		m_TargetNode = holder;
}

NodeState BehaviourTreeCallFunctionNode::OnExecute() {
	std::vector<const Variant *> args;
	args.reserve(m_Args.size());

	for (size_t i = 0; i < m_Args.size(); i++)
		args.emplace_back(&m_Args[i]);

	if (m_IsDeffered) {
		MessageQueue::get_singleton()->push_callp(m_TargetNode, m_FunctionName, args.data(), m_Args.size());
	} else {
		if (!m_IsRPC) {
			Callable::CallError err{};
			Variant ret = m_TargetNode->callp(m_FunctionName, args.data(), m_Args.size(), err);
			if (!m_ReturnValueName.is_empty())
				GetBehaviourTree()->SetBlackboard(m_ReturnValueName, ret);

			if (err.error != Callable::CallError::CALL_OK) {
#if TOOLS_ENABLED
				ERR_FAIL_V_MSG(NodeState::Failure, "Failed to call function " + m_FunctionName + " of node " + m_TargetNode->get_path());
#else
				return NodeState::Failure;
#endif
			}
		} else {
			m_TargetNode->rpcp(m_Args[0], m_FunctionName, args.data() + 1, args.size() - 1);
		}
	}

	return NodeState::Success;
}
} //namespace behaviour_tree::nodes
