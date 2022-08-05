#pragma once

#include "../action_node.hpp"
#include "../resources.hpp"

namespace behaviour_tree
{
	class BehaviourTreeResource;
}

namespace behaviour_tree::nodes
{
	class BehaviourTreeRefNode : public IBehaviourTreeActionNode
	{
		GDCLASS(BehaviourTreeRefNode, IBehaviourTreeActionNode);
	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("_set_tree", "tree"), &BehaviourTreeRefNode::SetTree);
			ClassDB::bind_method(D_METHOD("_get_tree"), &BehaviourTreeRefNode::GetTree);

			ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "behaviour_tree", PROPERTY_HINT_RESOURCE_TYPE, "BehaviourTreeResource"), "_set_tree", "_get_tree");

			ClassDB::bind_method(D_METHOD("_set_shared_blackboard", "tree"), &BehaviourTreeRefNode::SetSharedBlackboard);
			ClassDB::bind_method(D_METHOD("_get_shared_blackboard"), &BehaviourTreeRefNode::GetSharedBlackboard);

			ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shared_blackboard"), "_set_shared_blackboard", "_get_shared_blackboard");
		}

		void SerializeNode(Dictionary& out_data) const override
		{
			IBehaviourTreeActionNode::SerializeNode(out_data);
			out_data["behaviour_tree"] = m_Tree->get_path();
		}

		void DeserializeNode(const Dictionary& in_data)
		{
			m_Tree = ResourceLoader::load(in_data["behaviour_tree"], "BehaviourTreeResource");
			IBehaviourTreeActionNode::DeserializeNode(in_data);
		}

	public:
		void Rewind() override;

	protected:
		void OnEnter() override;
		NodeState OnExecute() override;

	private:
		void SetTree(const Ref<BehaviourTreeResource>& tree)
		{
			m_Tree = tree;
		}

		Ref<BehaviourTreeResource> GetTree()
		{
			return m_Tree;
		}

		void SetSharedBlackboard(bool shared)
		{

		}

		bool GetSharedBlackboard()
		{
			return false;
		}

	private:
		Ref<BehaviourTreeResource> m_Tree;
	};
}
