#pragma once

#include "../decorator_node.hpp"

namespace behaviour_tree::nodes
{
	class BehaviourTreeLoopNode : public IBehaviourTreeDecoratorNode
	{
		GDCLASS(BehaviourTreeLoopNode, IBehaviourTreeDecoratorNode);

	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("set_loop_count", "count"), &BehaviourTreeLoopNode::SetLoopCount, DEFVAL(-1));
			ClassDB::bind_method(D_METHOD("get_loop_count"), &BehaviourTreeLoopNode::GetLoopCount);
			ADD_PROPERTY(PropertyInfo(Variant::INT, "loop_count", PROPERTY_HINT_RANGE, "-1,99"), "set_loop_count", "get_loop_count");

			ClassDB::bind_method(D_METHOD("set_exit_on_failure", "state"), &BehaviourTreeLoopNode::SetExitOnFailure);
			ClassDB::bind_method(D_METHOD("get_exit_on_failure"), &BehaviourTreeLoopNode::GetExitOnFailure);
			ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exit_on_failure"), "set_exit_on_failure", "get_exit_on_failure");
		}

		void SerializeNode(Dictionary& out_data) const override
		{
			IBehaviourTreeDecoratorNode::SerializeNode(out_data);
			out_data["loop_count"] = m_LoopCount;
			out_data["exit_on_failure"] = m_ExitOnFailure;
		}

		void DeserializeNode(const Dictionary& in_data)
		{
			m_LoopCount = in_data["loop_count"];
			m_ExitOnFailure = in_data["exit_on_failure"];
			IBehaviourTreeDecoratorNode::DeserializeNode(in_data);
		}
	protected:
		void OnEnter() override
		{
			m_CurLoopCount = m_LoopCount;
		}

		NodeState OnExecute() override
		{
			if (m_CurLoopCount > 0 || m_CurLoopCount == -1)
			{
				m_Child->Execute();

				switch (m_Child->GetState())
				{
				case NodeState::Failure:
					if (m_ExitOnFailure)
						return NodeState::Failure;
					[[fallthrough]];

				case NodeState::Success:
					RestartChildrens();
					if (m_CurLoopCount != -1)
						--m_CurLoopCount;
					break;
				}

				return NodeState::Running;
			}
			return NodeState::Success;
		}

	private:
		void RestartChildrens()
		{
			BehaviourTree::Traverse(this,
				[](IBehaviourTreeNodeBehaviour* cur_node)
				{
					cur_node->Rewind();
				});
		}

		void SetLoopCount(int count = -1) noexcept
		{
			m_LoopCount = count;
		}
		
		int GetLoopCount() noexcept
		{
			return m_LoopCount;
		}

		void SetExitOnFailure(bool state) noexcept
		{
			m_ExitOnFailure = state;
		}

		bool GetExitOnFailure() noexcept
		{
			return m_ExitOnFailure;
		}

	private:
		int m_LoopCount = 2;
		bool m_ExitOnFailure = false;

		int m_CurLoopCount;
	};
}
