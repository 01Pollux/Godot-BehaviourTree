#include <queue>

#include "tree.hpp"
#include "factory.hpp"
#include "nodes/CustomNodes.hpp"

#include "decorator_node.hpp"
#include "nodes/AlwaysFailureNode.hpp"
#include "nodes/AlwaysSuccessNode.hpp"
#include "nodes/ConverterNode.hpp"
#include "nodes/LoopNode.hpp"
#include "nodes/TimeOutNode.hpp"

#include "action_node.hpp"
#include "nodes/BehaviourTreeRefNode.hpp"
#include "nodes/BreakPointNode.hpp"
#include "nodes/EmitSignalNode.hpp"
#include "nodes/PrintMessageNode.hpp"
#include "nodes/WaitTimeNode.hpp"

#include "composite_node.hpp"
#include "nodes/FallbackNode.hpp"
#include "nodes/SequenceNode.hpp"
#include "nodes/InterruptorNode.hpp"
#include "nodes/ParallelNode.hpp"
#include "nodes/RandomFallbackNode.hpp"
#include "nodes/RandomSequenceNode.hpp"

using namespace godot;

namespace behaviour_tree
{
	void BehaviourTree::_bind_methods()
	{
		ClassDB::bind_method(D_METHOD("execute_tree"), &BehaviourTree::ExecuteTree);
		ClassDB::bind_method(D_METHOD("set_always_running", "run_always"), &BehaviourTree::SetAlwaysRunning);

		ClassDB::bind_method(D_METHOD("rewind"), &BehaviourTree::Rewind);
		ClassDB::bind_method(D_METHOD("initialize_tree"), &BehaviourTree::Initialize);

		ClassDB::bind_method(D_METHOD("set_root", "root_node"), &BehaviourTree::SetRootNode);
		ClassDB::bind_method(D_METHOD("get_root"), &BehaviourTree::GDGetRootNode);

		ClassDB::bind_method(D_METHOD("create_node", "node_name"), &BehaviourTree::GDCreateNode);
		ClassDB::bind_method(D_METHOD("remove_node", "node"), &BehaviourTree::GDRemoveNode);
		ClassDB::bind_method(D_METHOD("remove_node_index", "node_index"), &BehaviourTree::GDRemoveNodeByIndex);
	}

	void BehaviourTree::register_types()
	{
		GDREGISTER_CLASS(BehaviourTree);
		GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeNodeBehaviour);

		GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeDecoratorNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeAlwaysFailureNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeAlwaysSuccessNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeConverterNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeLoopNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeTimeOutNode);

		GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeActionNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeRefNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeBreakPointNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeEmitSignalNode);
		GDREGISTER_CLASS(nodes::BehaviourTreePrintMessageNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeWaitTimeNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeCustomActionNode);
		// TODO: Add nodes::CallNode

		GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeCompositeNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeFallbackNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeInterruptorNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeParallelNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeRandomFallbackNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeRandomSequenceNode);
		GDREGISTER_CLASS(nodes::BehaviourTreeSequenceNode);

		BIND_CONSTANT(IBehaviourTreeNodeBehaviour::BEHAVIOUR_TREE_NODE_INACTIVE);
		BIND_CONSTANT(IBehaviourTreeNodeBehaviour::BEHAVIOUR_TREE_NODE_RUNNING);
		BIND_CONSTANT(IBehaviourTreeNodeBehaviour::BEHAVIOUR_TREE_NODE_SUCCESS);
		BIND_CONSTANT(IBehaviourTreeNodeBehaviour::BEHAVIOUR_TREE_NODE_FAILURE);
	}


	void BehaviourTree::Traverse(IBehaviourTreeNodeBehaviour* node, const std::function<void(IBehaviourTreeNodeBehaviour*)>& callback)
	{
		std::queue<IBehaviourTreeNodeBehaviour*> nodes;
		std::vector<IBehaviourTreeNodeBehaviour*> subnodes;

		IBehaviourTreeNodeBehaviour* cur_node = node;
		nodes.push(cur_node);

		while (!nodes.empty())
		{
			for (size_t i = 0; i < nodes.size(); i++)
			{
				IBehaviourTreeNodeBehaviour* cur_node = nodes.front();
				nodes.pop();

				callback(cur_node);

				cur_node->GetChildrens(subnodes);
				for (auto sub_child : subnodes)
					nodes.push(sub_child);
				subnodes.clear();
			}

			cur_node = nodes.front();
		}
	}

	void BehaviourTree::ExecuteTree()
	{
		ERR_FAIL_COND(m_RootNode.ptr() == nullptr);
		if (m_RootNode->GetState() < NodeState::SuccessOrFailure || m_RunAlways)
		{
			m_RootNode->Execute();
			if (m_RunAlways && m_RootNode->GetState() >= NodeState::SuccessOrFailure)
				Rewind();
		}
	}

	Ref<IBehaviourTreeNodeBehaviour> BehaviourTree::GetParentOfNode(IBehaviourTreeNodeBehaviour* node)
	{
		std::vector<IBehaviourTreeNodeBehaviour*> subnodes;
		for (auto& cur_node : m_Nodes)
		{
			if (cur_node == node)
				continue;

			cur_node->GetChildrens(subnodes);
			if (!subnodes.empty())
			{
				if (std::find(subnodes.begin(), subnodes.end(), node) != subnodes.end())
					return cur_node;
				subnodes.clear();
			}
		}
		return nullptr;
	}

	void BehaviourTree::Initialize()
	{
		Ref this_tree = this;
		for (auto& node : m_Nodes)
		{
			node->set_meta("behaviour_tree", this_tree);
			node->Initialize();
		}
	}

	void BehaviourTree::DisconnectConnectedNodes(IBehaviourTreeNodeBehaviour* node)
	{
		Ref parent_node = GetParentOfNode(node);
		if (parent_node.is_valid())
		{
			if (IBehaviourTreeCompositeNode* composite = Object::cast_to<IBehaviourTreeCompositeNode>(*parent_node))
				composite->RemoveChild(node);
			else
				Object::cast_to<IBehaviourTreeDecoratorNode>(*parent_node)->SetChild(nullptr);
		}
	}
}
