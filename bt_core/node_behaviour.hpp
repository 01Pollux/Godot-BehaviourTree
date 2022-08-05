#pragma once

#include <vector>
#include "core/io/resource.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/script_language.h"

namespace behaviour_tree
{
	enum class NodeState : char
	{
		Inactive = -1,
		Running,
		SuccessOrFailure,
		Success = SuccessOrFailure,
		Failure
	};

	class IBehaviourTreeNodeBehaviour : public Resource
	{
		GDCLASS(IBehaviourTreeNodeBehaviour, Resource);

	public:
		static void _bind_methods();

		enum BehaviourTreeNodeState
		{
			BEHAVIOUR_TREE_NODE_INACTIVE = -1,
			BEHAVIOUR_TREE_NODE_RUNNING,
			BEHAVIOUR_TREE_NODE_SUCCESS,
			BEHAVIOUR_TREE_NODE_FAILURE
		};

	public:
		NodeState Execute()
		{
			if (m_State == NodeState::Inactive)
				OnEnter();

			m_State = OnExecute();

			if (m_State != NodeState::Running)
				OnExit();

			return m_State;
		}

		template<typename _Ty = NodeState>
		_Ty GetState() const noexcept
		{
			return static_cast<_Ty>(m_State);
		}
		template<typename _Ty = NodeState>
		void SetState(_Ty state) noexcept
		{
			m_State = static_cast<NodeState>(state);
		}

	public:
		void Abort();

		virtual void Rewind()
		{
			m_State = NodeState::Inactive;
		}

		virtual bool GetChildrens(std::vector<IBehaviourTreeNodeBehaviour*>& childrens) = 0;
		virtual void Initialize()
		{}

		virtual void SerializeNode(Dictionary& out_data) const
		{}
		virtual void DeserializeNode(const Dictionary& in_data)
		{}

	protected:
		virtual void OnEnter()
		{}
		virtual void OnExit()
		{}
		virtual NodeState OnExecute() = 0;

	private:
		NodeState m_State{ NodeState::Inactive };
	};
}

VARIANT_ENUM_CAST(behaviour_tree::IBehaviourTreeNodeBehaviour::BehaviourTreeNodeState);
