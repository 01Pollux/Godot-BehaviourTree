
#include "../tree.hpp"
#include "CallFunctionNode.hpp"

namespace behaviour_tree::nodes {
void BehaviourTreeCallFunctionNode::Initialize() {
	m_Tree = get_meta("behaviour_tree");
	if (m_Tree.is_valid())
		m_TargetObject = ObjectDB::get_instance(m_Tree->get_meta("bt_node_object"));
}

NodeState BehaviourTreeCallFunctionNode::OnExecute() {
	m_TmpArgs.clear();
	m_TmpArgs.reserve(m_Args.size());

	for (size_t i = 0; i < m_Args.size(); i++)
		m_TmpArgs.emplace_back(&m_Args[i]);

	Callable::CallError err{};
	Variant ret = m_TargetObject->callp(m_Signal, m_TmpArgs.data(), m_Args.size(), err);
	m_Tree->set_meta("bt_last_return", ret);

	return NodeState::Success;
}
} //namespace behaviour_tree::nodes
