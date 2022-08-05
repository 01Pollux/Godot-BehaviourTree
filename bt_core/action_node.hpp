#pragma once

#include "node_behaviour.hpp"

namespace behaviour_tree
{
	class IBehaviourTreeActionNode : public IBehaviourTreeNodeBehaviour
	{
	public:
		bool GetChildrens(std::vector<IBehaviourTreeNodeBehaviour*>&) final
		{
			return false;
		}
	};
}
