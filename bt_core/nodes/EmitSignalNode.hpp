#pragma once

#include "../action_node.hpp"

namespace behaviour_tree
{
	class BehaviourTree;
}
namespace behaviour_tree::nodes
{
	class BehaviourTreeEmitSignalNode : public IBehaviourTreeActionNode
	{
		GDCLASS(BehaviourTreeEmitSignalNode, IBehaviourTreeActionNode);

	public:
		static void _bind_methods()
		{
			ClassDB::bind_method(D_METHOD("set_signal", "name"), &BehaviourTreeEmitSignalNode::SetSignal);
			ClassDB::bind_method(D_METHOD("get_signal"), &BehaviourTreeEmitSignalNode::GetSignal);
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "signal"), "set_signal", "get_signal");

			ClassDB::bind_method(D_METHOD("set_args", "args"), &BehaviourTreeEmitSignalNode::SetArgs);
			ClassDB::bind_method(D_METHOD("get_args"), &BehaviourTreeEmitSignalNode::GetArgs);
			ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "arguments"), "set_args", "get_args");
		}

		void Initialize() override;

		void SerializeNode(Dictionary& out_data) const override
		{
			IBehaviourTreeActionNode::SerializeNode(out_data);
			for (int i = 0; i < m_Args.size(); i++)
				out_data["args/" + String::num_int64(i)] = m_Args[i];

			out_data["signal"] = m_Signal;
		}

		void DeserializeNode(const Dictionary& in_data)
		{
			m_Args.clear();
			int i = 0;

			for (auto& arg : m_Args)
			{
				String key = "args/" + itos(i++);
				if (in_data.has(key))
					m_Args.push_back(key);
				else break;
			}

			m_Signal = in_data["signal"];
			IBehaviourTreeActionNode::DeserializeNode(in_data);
		}

	protected:
		NodeState OnExecute() override;

	private:
		void SetSignal(const String& signal)
		{
			m_Signal = signal;
		}

		String GetSignal() const
		{
			return m_Signal;
		}

		void SetArgs(const Vector<Variant>& args)
		{
			m_Args = args;
		}

		Vector<Variant> GetArgs() const
		{
			return m_Args;
		}

	private:
		Object* m_TargetObject;
		String m_Signal;
		Vector<Variant> m_Args;
		std::vector<const Variant*> m_TmpArgs;
	};
}
