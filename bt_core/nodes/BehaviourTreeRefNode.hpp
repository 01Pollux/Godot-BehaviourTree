#pragma once

#include "../action_node.hpp"
#include "../tree.hpp"

namespace behaviour_tree::nodes {
class BehaviourTreeRefNode : public IBehaviourTreeActionNode {
	GDCLASS(BehaviourTreeRefNode, IBehaviourTreeActionNode);

public:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_set_btree", "tree"), &BehaviourTreeRefNode::SetTree);
		ClassDB::bind_method(D_METHOD("_get_btree"), &BehaviourTreeRefNode::GetTree);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behaviour_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviourTree", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_STORAGE), "_set_btree", "_get_btree");
	}

	void SerializeNode(Dictionary &out_data) const override {
		IBehaviourTreeActionNode::SerializeNode(out_data);
		out_data["behaviour_tree"] = m_Tree.is_valid() ? m_Tree->get_path() : "<null>";
	}

	void DeserializeNode(const Dictionary &in_data) {
		String path = in_data["behaviour_tree"];
		if (path != "<null>")
			m_Tree = ResourceLoader::load(in_data["behaviour_tree"], "BehaviourTree");

		IBehaviourTreeActionNode::DeserializeNode(in_data);
	}

public:
	void Rewind() override;

protected:
	void OnEnter() override;
	NodeState OnExecute() override;

private:
	void SetTree(const Ref<BehaviourTree> &tree) {
		m_Tree = tree;
	}

	Ref<BehaviourTree> GetTree() {
		return m_Tree;
	}

private:
	Ref<BehaviourTree> m_Tree;
};
} //namespace behaviour_tree::nodes
