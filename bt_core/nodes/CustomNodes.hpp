#pragma once

#include "../tree.hpp"
#include "../action_node.hpp"
#include "../composite_node.hpp"
#include "../decorator_node.hpp"

namespace behaviour_tree::nodes {
class BehaviourTreeCustomActionNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeCustomActionNode, IBehaviourTreeActionNode);

public:
	static void _bind_methods();

protected:
	GDVIRTUAL0(_on_btnode_initialize);
	GDVIRTUAL0(_on_btnode_rewind);

	GDVIRTUAL0(_on_btnode_enter);
	GDVIRTUAL0R(BehaviourTree::BehaviourTreeNodeState, _on_btnode_execute);
	GDVIRTUAL0(_on_btnode_exit);

	GDVIRTUAL1RC(Dictionary, _on_btnode_serialize, Dictionary);
	GDVIRTUAL1(_on_btnode_deserialize, Dictionary);

public:
	void Rewind() override;
	void Initialize() override;

	void SerializeNode(Dictionary &out_data) const override;
	void DeserializeNode(const Dictionary &in_data) override;

protected:
	void OnEnter() override;
	NodeState OnExecute() override;
	void OnExit() override;
};

class BehaviourTreeCustomCompositeNode : public IBehaviourTreeCompositeNode {
	GDCLASS(BehaviourTreeCustomCompositeNode, IBehaviourTreeCompositeNode);

public:
	static void _bind_methods();

protected:
	GDVIRTUAL0(_on_btnode_initialize);
	GDVIRTUAL0(_on_btnode_rewind);

	GDVIRTUAL0(_on_btnode_enter);
	GDVIRTUAL0R(BehaviourTree::BehaviourTreeNodeState, _on_btnode_execute);
	GDVIRTUAL0(_on_btnode_exit);

	GDVIRTUAL1RC(Dictionary, _on_btnode_serialize, Dictionary);
	GDVIRTUAL1(_on_btnode_deserialize, Dictionary);

public:
	void Rewind() override;
	void Initialize() override;

	void SerializeNode(Dictionary &out_data) const override;
	void DeserializeNode(const Dictionary &in_data) override;

protected:
	void OnEnter() override;
	NodeState OnExecute() override;
	void OnExit() override;

private:
	Array GDGetChildrens();
	void GDSetChildrens(const Array &childrens);
};

class BehaviourTreeCustomDecoratorNode : public IBehaviourTreeDecoratorNode {
	GDCLASS(BehaviourTreeCustomDecoratorNode, IBehaviourTreeDecoratorNode);

public:
	static void _bind_methods();

protected:
	GDVIRTUAL0(_on_btnode_initialize);
	GDVIRTUAL0(_on_btnode_rewind);

	GDVIRTUAL0(_on_btnode_enter);
	GDVIRTUAL0R(BehaviourTree::BehaviourTreeNodeState, _on_btnode_execute);
	GDVIRTUAL0(_on_btnode_exit);

	GDVIRTUAL1RC(Dictionary, _on_btnode_serialize, Dictionary);
	GDVIRTUAL1(_on_btnode_deserialize, Dictionary);

public:
	void Rewind() override;
	void Initialize() override;

	void SerializeNode(Dictionary &out_data) const override;
	void DeserializeNode(const Dictionary &in_data) override;

protected:
	void OnEnter() override;
	NodeState OnExecute() override;
	void OnExit() override;
};
} //namespace behaviour_tree::nodes
