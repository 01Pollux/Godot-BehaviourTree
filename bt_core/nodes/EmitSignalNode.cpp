
#include "EmitSignalNode.hpp"
#include "../tree.hpp"

namespace behaviour_tree::nodes {
void BehaviourTreeEmitSignalNode::Initialize() {
	Node *holder = Object::cast_to<Node>(GetBehaviourTree()->GetBlackboard("bt_target_node"));
	if (holder && !m_TargetPath.is_empty())
		m_TargetNode = holder->get_node(m_TargetPath);
	else
		m_TargetNode = holder;
}

NodeState BehaviourTreeEmitSignalNode::OnExecute() {
	std::vector<const Variant *> args;
	args.reserve(m_Args.size());

	for (size_t i = 0; i < m_Args.size(); i++)
		args.emplace_back(&m_Args[i]);

	if (m_TargetNode->emit_signalp(m_SignalName, args.data(), m_Args.size()) != Error::OK) {
#if TOOLS_ENABLED
		ERR_FAIL_V_MSG(NodeState::Failure, "Failed to emit signal " + m_SignalName + " of node " + m_TargetNode->get_path());
#else
		return NodeState::Failure;
#endif
	}

	return NodeState::Success;
}
} //namespace behaviour_tree::nodes
