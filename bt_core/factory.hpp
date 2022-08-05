#pragma once

#include <memory>
#include <map>
#include "core/string/string_name.h"

namespace behaviour_tree
{
	class IBehaviourTreeNodeBehaviour;

	class BehaviourTreeFactory
	{
	public:
		using CreateNodeCallback = IBehaviourTreeNodeBehaviour*(*)();

		static std::unique_ptr<IBehaviourTreeNodeBehaviour> CreateNode(const StringName& node_name)
		{
			std::unique_ptr<IBehaviourTreeNodeBehaviour> node = nullptr;
			auto factory = m_Factories.find(node_name);

			if (factory != m_Factories.end())
				node = std::unique_ptr<IBehaviourTreeNodeBehaviour>((factory->second)());
			return node;
		}

	private:
		static inline std::map<StringName, CreateNodeCallback> m_Factories;
	};
}
