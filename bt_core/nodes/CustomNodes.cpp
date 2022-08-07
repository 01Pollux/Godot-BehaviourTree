#include "CustomNodes.hpp"

namespace behaviour_tree::nodes {
using namespace behaviour_tree;

void BehaviourTreeCustomActionNode::_bind_methods() {
	GDVIRTUAL_BIND(_on_btnode_rewind);
	GDVIRTUAL_BIND(_on_btnode_initialize);

	GDVIRTUAL_BIND(_on_btnode_serialize, "in_data");
	GDVIRTUAL_BIND(_on_btnode_deserialize, "out_data");

	GDVIRTUAL_BIND(_on_btnode_enter);
	GDVIRTUAL_BIND(_on_btnode_execute);
	GDVIRTUAL_BIND(_on_btnode_exit);
}

void BehaviourTreeCustomActionNode::Rewind() {
	IBehaviourTreeActionNode::Rewind();
	GDVIRTUAL_CALL(_on_btnode_rewind);
}

void BehaviourTreeCustomActionNode::Initialize() {
	GDVIRTUAL_CALL(_on_btnode_initialize);
}

void BehaviourTreeCustomActionNode::SerializeNode(Dictionary &out_data) const {
	IBehaviourTreeActionNode::SerializeNode(out_data);

	List<PropertyInfo> props;
	get_property_list(&props);
	for (auto &prop : props) {
		if ((prop.usage & (PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE)) == (PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE))
			out_data[prop.name] = get(prop.name);
	}

	GDVIRTUAL_CALL(_on_btnode_serialize, out_data, out_data);
}

void BehaviourTreeCustomActionNode::DeserializeNode(const Dictionary &in_data) {
	IBehaviourTreeActionNode::DeserializeNode(in_data);

	List<PropertyInfo> props;
	get_property_list(&props);
	for (auto &prop : props) {
		if ((prop.usage & (PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE)) == (PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_SCRIPT_VARIABLE) &&
				in_data.has(prop.name))
			set(prop.name, in_data[prop.name]);
	}

	GDVIRTUAL_CALL(_on_btnode_deserialize, in_data);
}

void BehaviourTreeCustomActionNode::OnEnter() {
	GDVIRTUAL_CALL(_on_btnode_enter);
}

NodeState BehaviourTreeCustomActionNode::OnExecute() {
	BehaviourTree::BehaviourTreeNodeState ret = BehaviourTree::BEHAVIOUR_TREE_NODE_INACTIVE;
	GDVIRTUAL_CALL(_on_btnode_execute, ret);
	return static_cast<NodeState>(ret);
}

void BehaviourTreeCustomActionNode::OnExit() {
	GDVIRTUAL_CALL(_on_btnode_exit);
}

void BehaviourTreeCustomCompositeNode::_bind_methods() {
	ClassDB::bind_method("get_childrens", &BehaviourTreeCustomCompositeNode::GDGetChildrens);
	ClassDB::bind_method("set_childrens", &BehaviourTreeCustomCompositeNode::GDSetChildrens);

	GDVIRTUAL_BIND(_on_btnode_rewind);
	GDVIRTUAL_BIND(_on_btnode_initialize);

	GDVIRTUAL_BIND(_on_btnode_serialize, "in_data");
	GDVIRTUAL_BIND(_on_btnode_deserialize, "out_data");

	GDVIRTUAL_BIND(_on_btnode_enter);
	GDVIRTUAL_BIND(_on_btnode_execute);
	GDVIRTUAL_BIND(_on_btnode_exit);
}

void BehaviourTreeCustomCompositeNode::Rewind() {
	IBehaviourTreeCompositeNode::Rewind();
	GDVIRTUAL_CALL(_on_btnode_rewind);
}

void BehaviourTreeCustomCompositeNode::Initialize() {
	GDVIRTUAL_CALL(_on_btnode_initialize);
}

void BehaviourTreeCustomCompositeNode::SerializeNode(Dictionary &out_data) const {
	IBehaviourTreeCompositeNode::SerializeNode(out_data);

	List<PropertyInfo> props;
	get_property_list(&props);
	for (auto &prop : props) {
		if (prop.usage & PROPERTY_USAGE_SCRIPT_VARIABLE)
			out_data[prop.name] = get(prop.name);
	}

	GDVIRTUAL_CALL(_on_btnode_serialize, out_data, out_data);
}

void BehaviourTreeCustomCompositeNode::DeserializeNode(const Dictionary &in_data) {
	IBehaviourTreeCompositeNode::DeserializeNode(in_data);

	List<PropertyInfo> props;
	get_property_list(&props);
	for (auto &prop : props) {
		if (prop.usage & PROPERTY_USAGE_SCRIPT_VARIABLE && in_data.has(prop.name))
			set(prop.name, in_data[prop.name]);
	}

	GDVIRTUAL_CALL(_on_btnode_deserialize, in_data);
}

void BehaviourTreeCustomCompositeNode::OnEnter() {
	GDVIRTUAL_CALL(_on_btnode_enter);
}

NodeState BehaviourTreeCustomCompositeNode::OnExecute() {
	BehaviourTree::BehaviourTreeNodeState ret = BehaviourTree::BEHAVIOUR_TREE_NODE_INACTIVE;
	GDVIRTUAL_CALL(_on_btnode_execute, ret);
	return static_cast<NodeState>(ret);
}

void BehaviourTreeCustomCompositeNode::OnExit() {
	GDVIRTUAL_CALL(_on_btnode_exit);
}

Array BehaviourTreeCustomCompositeNode::GDGetChildrens() {
	Array childrens;
	childrens.resize(m_Childrens.size());
	for (size_t i = 0; i < childrens.size(); i++)
		childrens[i] = m_Childrens[i];
	return childrens;
}

void BehaviourTreeCustomCompositeNode::GDSetChildrens(const Array &childrens) {
	m_Childrens.clear();
	m_Childrens.reserve(childrens.size());
	for (size_t i = 0; i < childrens.size(); i++)
		m_Childrens.emplace_back(childrens[i]);
}

void BehaviourTreeCustomDecoratorNode::_bind_methods() {
	GDVIRTUAL_BIND(_on_btnode_rewind);
	GDVIRTUAL_BIND(_on_btnode_initialize);

	GDVIRTUAL_BIND(_on_btnode_serialize, "in_data");
	GDVIRTUAL_BIND(_on_btnode_deserialize, "out_data");

	GDVIRTUAL_BIND(_on_btnode_enter);
	GDVIRTUAL_BIND(_on_btnode_execute);
	GDVIRTUAL_BIND(_on_btnode_exit);
}

void BehaviourTreeCustomDecoratorNode::Rewind() {
	IBehaviourTreeDecoratorNode::Rewind();
	GDVIRTUAL_CALL(_on_btnode_rewind);
}

void BehaviourTreeCustomDecoratorNode::Initialize() {
	GDVIRTUAL_CALL(_on_btnode_initialize);
}

void BehaviourTreeCustomDecoratorNode::SerializeNode(Dictionary &out_data) const {
	IBehaviourTreeDecoratorNode::SerializeNode(out_data);

	List<PropertyInfo> props;
	get_property_list(&props);
	for (auto &prop : props) {
		if (prop.usage & PROPERTY_USAGE_SCRIPT_VARIABLE)
			out_data[prop.name] = get(prop.name);
	}

	GDVIRTUAL_CALL(_on_btnode_serialize, out_data, out_data);
}

void BehaviourTreeCustomDecoratorNode::DeserializeNode(const Dictionary &in_data) {
	IBehaviourTreeDecoratorNode::DeserializeNode(in_data);

	List<PropertyInfo> props;
	get_property_list(&props);
	for (auto &prop : props) {
		if (prop.usage & PROPERTY_USAGE_SCRIPT_VARIABLE && in_data.has(prop.name))
			set(prop.name, in_data[prop.name]);
	}

	GDVIRTUAL_CALL(_on_btnode_deserialize, in_data);
}

void BehaviourTreeCustomDecoratorNode::OnEnter() {
	GDVIRTUAL_CALL(_on_btnode_enter);
}

NodeState BehaviourTreeCustomDecoratorNode::OnExecute() {
	BehaviourTree::BehaviourTreeNodeState ret = BehaviourTree::BEHAVIOUR_TREE_NODE_INACTIVE;
	GDVIRTUAL_CALL(_on_btnode_execute, ret);
	return static_cast<NodeState>(ret);
}

void BehaviourTreeCustomDecoratorNode::OnExit() {
	GDVIRTUAL_CALL(_on_btnode_exit);
}
} //namespace behaviour_tree::nodes
