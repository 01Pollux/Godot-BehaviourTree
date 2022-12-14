#include <queue>

#include "nodes/CustomNodes.hpp"
#include "tree.hpp"

#include "decorator_node.hpp"
#include "nodes/AlwaysFailureNode.hpp"
#include "nodes/AlwaysSuccessNode.hpp"
#include "nodes/ConverterNode.hpp"
#include "nodes/LoopNode.hpp"
#include "nodes/TimeOutNode.hpp"

#include "action_node.hpp"
#include "nodes/BehaviourTreeRefNode.hpp"
#include "nodes/BreakPointNode.hpp"
#include "nodes/CallFunctionNode.hpp"
#include "nodes/EmitSignalNode.hpp"
#include "nodes/PrintMessageNode.hpp"
#include "nodes/WaitTimeNode.hpp"

#include "composite_node.hpp"
#include "nodes/FallbackNode.hpp"
#include "nodes/InterruptorNode.hpp"
#include "nodes/ParallelNode.hpp"
#include "nodes/RandomFallbackNode.hpp"
#include "nodes/RandomSequenceNode.hpp"
#include "nodes/SequenceNode.hpp"

using namespace godot;

namespace behaviour_tree {
void BehaviourTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("execute_tree"), &BehaviourTree::ExecuteTree);
	ClassDB::bind_method(D_METHOD("set_always_running", "run_always"), &BehaviourTree::SetAlwaysRunning);

	ClassDB::bind_method(D_METHOD("rewind"), &BehaviourTree::Rewind);
	ClassDB::bind_method(D_METHOD("initialize_tree"), &BehaviourTree::InitializeTree);

	ClassDB::bind_method(D_METHOD("set_root", "root_node"), &BehaviourTree::SetRootNode);
	ClassDB::bind_method(D_METHOD("get_root"), &BehaviourTree::GDGetRootNode);

	ClassDB::bind_method(D_METHOD("create_node", "node_name"), &BehaviourTree::GDCreateNode);
	ClassDB::bind_method(D_METHOD("remove_node", "node"), &BehaviourTree::GDRemoveNode);
	ClassDB::bind_method(D_METHOD("remove_node_index", "node_index"), &BehaviourTree::GDRemoveNodeByIndex);

	ClassDB::bind_method(D_METHOD("_set_bt_root_index", "root_node"), &BehaviourTree::GDSetRootNodeIndex);
	ClassDB::bind_method(D_METHOD("_get_bt_root_index"), &BehaviourTree::GDGetRootNodeIndex);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "_bt_root_index", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_bt_root_index", "_get_bt_root_index");

	ClassDB::bind_method(D_METHOD("_set_bt_nodes", "nodes"), &BehaviourTree::GDSetNodes);
	ClassDB::bind_method(D_METHOD("_get_bt_nodes"), &BehaviourTree::GDGetNodes);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "_bt_nodes", PROPERTY_HINT_ARRAY_TYPE, "", PROPERTY_USAGE_STORAGE), "_set_bt_nodes", "_get_bt_nodes");
	
	ClassDB::bind_method(D_METHOD("set_blackboard", "key", "data"), &BehaviourTree::SetBlackboard);
	ClassDB::bind_method(D_METHOD("get_blackboard", "key"), &BehaviourTree::GetBlackboard);

#if TOOLS_ENABLED
	ADD_SIGNAL(MethodInfo("_on_btree_execute"));
#endif
}

void BehaviourTree::register_types() {
	GDREGISTER_CLASS(BehaviourTree);
	GDREGISTER_CLASS(BehaviourTreeHolder);
	GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeNodeBehaviour);

	GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeDecoratorNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeAlwaysFailureNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeAlwaysSuccessNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeConverterNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeLoopNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeTimeOutNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeCustomDecoratorNode);

	GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeActionNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeRefNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeBreakPointNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeCallFunctionNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeEmitSignalNode);
	GDREGISTER_CLASS(nodes::BehaviourTreePrintMessageNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeWaitTimeNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeCustomActionNode);

	GDREGISTER_ABSTRACT_CLASS(IBehaviourTreeCompositeNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeFallbackNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeInterruptorNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeParallelNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeRandomFallbackNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeRandomSequenceNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeSequenceNode);
	GDREGISTER_CLASS(nodes::BehaviourTreeCustomCompositeNode);

	BIND_CONSTANT(BEHAVIOUR_TREE_NODE_INACTIVE);
	BIND_CONSTANT(BEHAVIOUR_TREE_NODE_RUNNING);
	BIND_CONSTANT(BEHAVIOUR_TREE_NODE_SUCCESS);
	BIND_CONSTANT(BEHAVIOUR_TREE_NODE_FAILURE);

	BTreeResLoader.instantiate();
	BTreeResSaver.instantiate();

	ResourceLoader::add_resource_format_loader(BTreeResLoader);
	ResourceSaver::add_resource_format_saver(BTreeResSaver);
}

void BehaviourTree::unregister_types() {
	ResourceLoader::remove_resource_format_loader(BTreeResLoader);
	ResourceSaver::remove_resource_format_saver(BTreeResSaver);

	BTreeResLoader.unref();
	BTreeResSaver.unref();
}

void BehaviourTree::reset_state() {
	Rewind();
}

void BehaviourTree::Traverse(IBehaviourTreeNodeBehaviour *node, const std::function<void(IBehaviourTreeNodeBehaviour *)> &callback) {
	std::queue<IBehaviourTreeNodeBehaviour *> nodes;
	std::vector<IBehaviourTreeNodeBehaviour *> subnodes;

	IBehaviourTreeNodeBehaviour *cur_node = node;
	nodes.push(cur_node);

	while (!nodes.empty()) {
		for (size_t i = 0; i < nodes.size(); i++) {
			IBehaviourTreeNodeBehaviour *cur_node = nodes.front();
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

void BehaviourTree::ExecuteTree() {
	IBehaviourTreeNodeBehaviour *root = GetRootNode();
	ERR_FAIL_COND(root == nullptr);
	if (root->GetState() < NodeState::SuccessOrFailure || m_RunAlways) {
		root->Execute();
#if TOOLS_ENABLED
		emit_signal("_on_btree_execute");
#endif
		if (m_RunAlways && root->GetState() >= NodeState::SuccessOrFailure)
			Rewind();
	}
}

Ref<IBehaviourTreeNodeBehaviour> BehaviourTree::GetParentOfNode(IBehaviourTreeNodeBehaviour *node) {
	std::vector<IBehaviourTreeNodeBehaviour *> subnodes;
	for (auto &cur_node : m_Nodes) {
		if (cur_node == node)
			continue;

		cur_node->GetChildrens(subnodes);
		if (!subnodes.empty()) {
			if (std::find(subnodes.begin(), subnodes.end(), node) != subnodes.end())
				return cur_node;
			subnodes.clear();
		}
	}
	return nullptr;
}

Ref<Resource> BehaviourTree::duplicate(bool) const {
	Ref<BehaviourTree> copy;

	copy.instantiate();
	copy->SetAlwaysRunning(IsAlwaysRunning());
	copy->GDSetRootNodeIndex(GDGetRootNodeIndex());

	std::map<const IBehaviourTreeNodeBehaviour *, Ref<IBehaviourTreeNodeBehaviour>> final_nodes;

	// Load the new nodes and root nodes
	for (size_t i = 0; i < m_Nodes.size(); i++) {
		auto &node = m_Nodes[i];
		Ref new_node = node->duplicate(true);

		final_nodes.emplace(*node, new_node);
		copy->m_Nodes.emplace_back(new_node);
	}

	// remap the node's childrens
	std::vector<IBehaviourTreeNodeBehaviour *> subnodes;
	for (auto &[cur_node, remap_parent] : final_nodes) {
		cur_node->GetChildrens(subnodes);
		if (!subnodes.empty()) {
			if (auto new_composite = Object::cast_to<IBehaviourTreeCompositeNode>(*remap_parent)) {
				auto &childrens = new_composite->GetChildrens();
				childrens.clear();

				for (auto &sub_node : subnodes) {
					childrens.emplace_back(final_nodes[sub_node]);
				}
			} else if (auto new_decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(*remap_parent)) {
				new_decorator->SetChild(final_nodes[subnodes[0]]);
			}
			subnodes.clear();
		}
	}

	return copy;
}

void BehaviourTree::DisconnectConnectedNodes(IBehaviourTreeNodeBehaviour *node) {
	Ref parent_node = GetParentOfNode(node);
	if (parent_node.is_valid()) {
		if (IBehaviourTreeCompositeNode *composite = Object::cast_to<IBehaviourTreeCompositeNode>(*parent_node))
			composite->RemoveChild(node);
		else
			Object::cast_to<IBehaviourTreeDecoratorNode>(*parent_node)->SetChild(nullptr);
	}
}

void BehaviourTree::setup_local_to_scene() {
	if (m_IsUnique || !get_local_scene())
		return;

	m_IsUnique = true;
	std::map<Ref<IBehaviourTreeNodeBehaviour>, IBehaviourTreeNodeBehaviour *> final_nodes;

	for (auto &node : m_Nodes) {
		Ref<IBehaviourTreeNodeBehaviour> new_node = node->duplicate(true);
		final_nodes.emplace(node, *new_node);
		node = new_node;
		node->SetBehaviourTree(this);
	}

	// remap the node's childrens
	std::vector<IBehaviourTreeNodeBehaviour *> subnodes;
	for (auto &[cur_node, remap_parent] : final_nodes) {
		cur_node->GetChildrens(subnodes);
		if (!subnodes.empty()) {
			if (auto new_composite = Object::cast_to<IBehaviourTreeCompositeNode>(remap_parent)) {
				auto &childrens = new_composite->GetChildrens();
				childrens.clear();

				for (auto &sub_node : subnodes) {
					childrens.emplace_back(final_nodes[sub_node]);
				}
			} else if (auto new_decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(remap_parent)) {
				new_decorator->SetChild(final_nodes[subnodes[0]]);
			}
			subnodes.clear();
		}
	}
}

void BehaviourTreeHolder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_set_target_node"), &BehaviourTreeHolder::SetTargetPath);
	ClassDB::bind_method(D_METHOD("_get_target_node"), &BehaviourTreeHolder::GetTargetPath);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node"), "_set_target_node", "_get_target_node");

	ClassDB::bind_method(D_METHOD("_set_btree", "tree"), &BehaviourTreeHolder::SetBehaviourTreeRes);
	ClassDB::bind_method(D_METHOD("_get_btree"), &BehaviourTreeHolder::GetBehaviourTree);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behaviour_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviourTree"), "_set_btree", "_get_btree");

	ClassDB::bind_method(D_METHOD("execute_tree"), &BehaviourTreeHolder::ExecuteTree);
}

void BehaviourTreeHolder::_notification(int p_notification) {
	if (!SceneTree::get_singleton()->get_current_scene())
		return;
	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			if (m_Tree.is_valid()) {
				m_Tree->SetBlackboard("bt_target_node", get_node(GetTargetPath()));
			}
			break;
		}
		case NOTIFICATION_READY: {
			if (m_Tree.is_valid())
				m_Tree->InitializeTree();
			break;
		}
	}
}
} //namespace behaviour_tree
