
#if TOOLS_ENABLED
#include "CallFunctionNodeEditorPlugin.hpp"
#include "../../nodes/CallFunctionNode.hpp"

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/editor_properties.h"
#include "editor/editor_scale.h"

#include "editor/create_dialog.h"
#include "editor/property_selector.h"
#include "scene/gui/box_container.h"

using namespace behaviour_tree::nodes;

namespace behaviour_tree::editor {
class CallFunctionNodeEditorProp : public EditorProperty {
	GDCLASS(CallFunctionNodeEditorProp, EditorProperty);

public:
	CallFunctionNodeEditorProp();

	void Setup(const Ref<BehaviourTreeCallFunctionNode> &func_call);

private:
	void OnFunctionSelected(const String &name);
	void OnFunctionPressed();

private:
	Ref<BehaviourTreeCallFunctionNode> m_CallFuncNode;
	ObjectID m_TargetNode;
	PropertySelector *m_MethodSelector = nullptr;
	Button *m_PropertyButton = nullptr;
};

class CallFunctionNodeInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(CallFunctionNodeInspectorPlugin, EditorInspectorPlugin);

public:
	bool can_handle(Object *p_object) override {
		return Object::cast_to<BehaviourTreeCallFunctionNode>(p_object) != nullptr;
	}

	bool parse_property(
		Object *p_object,
		const Variant::Type p_type,
		const String &p_path,
		const PropertyHint p_hint,
		const String &p_hint_text,
		const uint32_t p_usage,
		const bool p_wide
	) override {
		if (p_path == "callback_function") {
			auto call_func = Object::cast_to<BehaviourTreeCallFunctionNode>(p_object);
			if (!call_func)
				return false;

			CallFunctionNodeEditorProp *prop = memnew(CallFunctionNodeEditorProp);
			prop->Setup(call_func);
			add_property_editor("callback", prop);

			return true;
		}
		return false;
	}
};

void CallFunctionNodeEditorProp::Setup(const Ref<BehaviourTreeCallFunctionNode> &node) {
	m_CallFuncNode = node;
	String callback = m_CallFuncNode->GetCallbackFunction();
	if (callback.is_empty())
		callback = "<callback>";
	m_PropertyButton->set_text(callback);
}

CallFunctionNodeEditorProp::CallFunctionNodeEditorProp() {
	m_MethodSelector = memnew(PropertySelector);
	add_child(m_MethodSelector);
	m_MethodSelector->connect("selected", callable_mp(this, &CallFunctionNodeEditorProp::OnFunctionSelected));

	m_PropertyButton = memnew(Button);
	m_PropertyButton->set_clip_text(true);
	add_child(m_PropertyButton);
	add_focusable(m_PropertyButton);
	m_PropertyButton->connect("pressed", callable_mp(this, &CallFunctionNodeEditorProp::OnFunctionPressed));
}

void CallFunctionNodeEditorProp::OnFunctionSelected(const String &name) {
	Node *target_node = Object::cast_to<Node>(ObjectDB::get_instance(m_TargetNode));
	if (!target_node)
		return;

	List<MethodInfo> minfo;
	target_node->get_method_list(&minfo);

	for (const MethodInfo &info : minfo) {
		if (info.name == name) {
			Array params;
			int first_defarg = info.arguments.size() - info.default_arguments.size();

			for (int i = 0; i < info.arguments.size(); i++) {
				if (i >= first_defarg) {
					Variant arg = info.default_arguments[i - first_defarg];
					params.push_back(arg);
				} else {
					Callable::CallError ce;
					Variant arg;
					Variant::construct(info.arguments[i].type, arg, nullptr, 0, ce);
					params.push_back(arg);
				}
			}

			UndoRedo *undo_redo = EditorNode::get_undo_redo();
			undo_redo->create_action(TTR("Add Callback Function"));

			undo_redo->add_do_method(*m_CallFuncNode, "set_callback_function", name);
			undo_redo->add_do_method(m_PropertyButton, "set_text", name);
			undo_redo->add_do_method(*m_CallFuncNode, "set_args", params);

			String old_name = m_CallFuncNode->GetCallbackFunction();
			if (old_name.is_empty())
				old_name = "<function>";

			undo_redo->add_undo_method(*m_CallFuncNode, "set_callback_function", old_name);
			undo_redo->add_do_method(m_PropertyButton, "set_text", old_name);
			undo_redo->add_undo_method(*m_CallFuncNode, "set_args", m_CallFuncNode->GetArgs());

			undo_redo->commit_action();
			return;
		}
	}

	EditorNode::get_singleton()->show_warning(TTR("Method not found in object:") + " " + name);
}

void CallFunctionNodeEditorProp::OnFunctionPressed() {
	Node *root = SceneTree::get_singleton()->get_edited_scene_root();
	ERR_FAIL_COND_MSG(root == nullptr, "no current root node is being edited");

	NodePath path = m_CallFuncNode->GetTargetNode();
	ERR_FAIL_COND_MSG(path.is_empty(), "Invalid target node was selected");

	Node *target_node = root->get_node(path);
	ERR_FAIL_COND_MSG(target_node == nullptr, "Invalid target node was selected");

	m_TargetNode = target_node->get_instance_id();
	m_MethodSelector->select_method_from_instance(target_node);
}

CallFunctionNodeEditorPlugin::CallFunctionNodeEditorPlugin() {
	add_inspector_plugin(memnew(CallFunctionNodeInspectorPlugin));
}

} //namespace behaviour_tree::editor

#endif
