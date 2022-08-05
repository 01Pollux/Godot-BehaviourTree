#pragma once

#include "../composite_node.hpp"

namespace behaviour_tree::nodes
{
	class BehaviourTreeSequenceNode : public IBehaviourTreeCompositeNode
	{
		GDCLASS(BehaviourTreeSequenceNode, IBehaviourTreeCompositeNode);

	protected:
		NodeState OnExecute() override
		{
			for (size_t i = 0; i < m_Childrens.size(); i++)
			{
				if (m_Childrens[i]->GetState() != NodeState::Success)
				{
					switch (m_Childrens[i]->Execute())
					{
					case NodeState::Success:
						continue;

					case NodeState::Failure:
						return NodeState::Failure;
					}
					return NodeState::Running;
				}
			}
			return NodeState::Success;
		}
	};
}
