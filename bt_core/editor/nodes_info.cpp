#include "../visual_resources.hpp"
#include "editor.hpp"

#include "../nodes/CustomNodes.hpp"

#if TOOLS_ENABLED
namespace behaviour_tree::editor {
void BehaviourTreeViewer::InitializeNodesInfo() {
	m_RegisteredNodesInfo.emplace_back("Sequence", "Common/Composites", "BehaviourTreeSequenceNode", "Executes the childrens from top to bottom and fails if any of them fails");
	m_RegisteredNodesInfo.emplace_back("Parallel", "Common/Composites", "BehaviourTreeParallelNode", "Executes the first two of childrens regardless of the previous state");
	m_RegisteredNodesInfo.emplace_back("Fallback", "Common/Composites", "BehaviourTreeFallbackNode", "Execute childrens from to bottom and immediatly succeed if any of them succeed");
	m_RegisteredNodesInfo.emplace_back("Interruptor", "Common/Composites", "BehaviourTreeInterruptorNode", "Executes the childrens and fails the rest in case any of them didn't fail");
	m_RegisteredNodesInfo.emplace_back("Random Sequence", "Common/Composites", "BehaviourTreeRandomSequenceNode", "Execute childrens in random order and fails if any of them fails");
	m_RegisteredNodesInfo.emplace_back("Random fallback", "Common/Composites", "BehaviourTreeRandomFallbackNode", "Execute childrens in random order and immediatly succeed if any of them succeed");

	m_RegisteredNodesInfo.emplace_back("Always Success", "Common/Decorators", "BehaviourTreeAlwaysSuccessNode", "Forces success state");
	m_RegisteredNodesInfo.emplace_back("Always Failure", "Common/Decorators", "BehaviourTreeAlwaysFailureNode", "Forces failure state");
	m_RegisteredNodesInfo.emplace_back("Converter", "Common/Decorators", "BehaviourTreeConverterNode", "Mutate the upcoming node state");
	m_RegisteredNodesInfo.emplace_back("Timeout", "Common/Decorators", "BehaviourTreeTimeOutNode", "Terminate execution if the wait time has exceeded");

	m_RegisteredNodesInfo.emplace_back("Emit Signal", "Common/Functions", "BehaviourTreeEmitSignalNode", "Emit a signal from current 'bt_node_object' in blackboard");
	m_RegisteredNodesInfo.emplace_back("Call Function", "Common/Functions", "BehaviourTreeCallFunctionNode", "Call a function from current 'bt_node_object' in blackboard");

	m_RegisteredNodesInfo.emplace_back("Wait Time", "Common/Actions", "BehaviourTreeWaitTimeNode", "Suspend execution for set period of time");
	m_RegisteredNodesInfo.emplace_back("Loop", "Common/Actions", "LoopNode", "Loops on execution of a node");
	m_RegisteredNodesInfo.emplace_back("Tree reference", "Common/Actions", "BehaviourTreeRefNode", "References an external behaviour tree");

	m_RegisteredNodesInfo.emplace_back("Print", "Common/Debug", "BehaviourTreePrintMessageNode", "Print a message to the console");
	m_RegisteredNodesInfo.emplace_back("Break point", "Common/Debug", "BehaviourTreeBreakPointNode", "Pauses the game");
}

void BehaviourTreeViewer::UpdateCustomNodesInfo() {
	m_CustomNodesInfo.clear();

	List<StringName> class_list;
	ScriptServer::get_global_class_list(&class_list);

	for (String cur_class : class_list) {
		Dictionary cur_node_info = m_VisualTreeHolder->GetCustomNodeInfo(cur_class);
		if (cur_node_info.is_empty())
			continue;

		StringName native_class = ScriptServer::get_global_class_native_base(cur_class);
		if (!(native_class == "BehaviourTreeCustomActionNode" ||
					native_class == "BehaviourTreeCustomCompositeNode" ||
					native_class == "BehaviourTreeCustomDecoratorNode"))
			continue;

		String script_path = ScriptServer::get_global_class_path(cur_class);
		Ref<Script> script = ResourceLoader::load(script_path);
		ERR_FAIL_COND(script.is_null());

		Ref<nodes::BehaviourTreeCustomActionNode> custom_node;
		custom_node.instantiate();
		custom_node->set_script(*script);

		m_CustomNodesInfo.emplace_back(
				cur_node_info.has("name") ? cur_node_info["name"] : "",
				cur_node_info.has("category") ? cur_node_info["category"] : "",
				cur_class,
				cur_node_info.has("description") ? cur_node_info["description"] : "",
				script);
	}
}
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
