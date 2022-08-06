
#include "EmitSignalNode.hpp"
#include "../tree.hpp"

namespace behaviour_tree::nodes {
void BehaviourTreeEmitSignalNode::Initialize() {
	Ref<BehaviourTree> tree = get_meta("behaviour_tree");
	if (tree.is_valid())
		m_TargetObject = ObjectDB::get_instance(tree->get_meta("bt_node_object"));
}

NodeState BehaviourTreeEmitSignalNode::OnExecute() {
	m_TmpArgs.clear();
	m_TmpArgs.reserve(m_Args.size());

	for (size_t i = 0; i < m_Args.size(); i++)
		m_TmpArgs.emplace_back(&m_Args[i]);

	m_TargetObject->emit_signalp(m_Signal, m_TmpArgs.data(), m_Args.size());

	return NodeState::Success;
}
} //namespace behaviour_tree::nodes
