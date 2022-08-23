
#if TOOLS_ENABLED
#include "EmitSignalNodeEditorPlugin.hpp"
#include "../../nodes/EmitSignalNode.hpp"

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/editor_properties.h"
#include "editor/editor_scale.h"

#include "editor/create_dialog.h"
#include "editor/property_selector.h"
#include "scene/gui/box_container.h"

using namespace behaviour_tree::nodes;

namespace behaviour_tree::editor {
class EmitSignalNodeEditorProp : public EditorProperty {
	GDCLASS(EmitSignalNodeEditorProp, EditorProperty);

public:
	EmitSignalNodeEditorProp();

	void Setup(const Ref<BehaviourTreeEmitSignalNode> &func_call);

private:
	void SetupNode(const Ref<BehaviourTreeEmitSignalNode> &node);
	void SetupOptions();

	void OnSignalSelected(int id);
	void OnPathChanged();

private:
	Ref<BehaviourTreeEmitSignalNode> m_SignalNode;
	ObjectID m_TargetNode;
	OptionButton *m_SignalSelector = nullptr;
};

class EmitSignalInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(EmitSignalInspectorPlugin, EditorInspectorPlugin);

public:
	bool can_handle(Object *p_object) override {
		return Object::cast_to<BehaviourTreeEmitSignalNode>(p_object) != nullptr;
	}

	bool parse_property(
			Object *p_object,
			const Variant::Type p_type,
			const String &p_path,
			const PropertyHint p_hint,
			const String &p_hint_text,
			const uint32_t p_usage,
			const bool p_wide) override {
		if (p_path == "signal_name") {
			auto call_func = Object::cast_to<BehaviourTreeEmitSignalNode>(p_object);
			if (!call_func)
				return false;

			EmitSignalNodeEditorProp *prop = memnew(EmitSignalNodeEditorProp);
			prop->Setup(call_func);
			add_property_editor(p_path, prop);

			return true;
		}
		return false;
	}
};

void EmitSignalNodeEditorProp::Setup(const Ref<BehaviourTreeEmitSignalNode> &node) {
	SetupNode(node);
	
	m_SignalNode = node;
	m_SignalNode->connect("_btree_signal_node_path_changed", callable_mp(this, &EmitSignalNodeEditorProp::OnPathChanged));
	m_SignalSelector->connect("item_selected", callable_mp(this, &EmitSignalNodeEditorProp::OnSignalSelected));

	SetupOptions();
}

EmitSignalNodeEditorProp::EmitSignalNodeEditorProp() {
	m_SignalSelector = memnew(OptionButton);
	m_SignalSelector->set_clip_text(true);
	add_child(m_SignalSelector);
	add_focusable(m_SignalSelector);
}

void EmitSignalNodeEditorProp::SetupNode(const Ref<BehaviourTreeEmitSignalNode> &node) {
	Node *root = SceneTree::get_singleton()->get_edited_scene_root();
	if (!root)
		return;

	NodePath path = node->GetTargetNode();
	if (path.is_empty())
		return;

	Node *target_node = root->get_node(path);
	if (!target_node)
		return;

	m_TargetNode = target_node->get_instance_id();
}

void EmitSignalNodeEditorProp::SetupOptions() {
	Node *target_node = Object::cast_to<Node>(ObjectDB::get_instance(m_TargetNode));
	if (!target_node)
		return;

	List<MethodInfo> minfos;
	target_node->get_signal_list(&minfos);
	String signal = m_SignalNode->GetCallbackSignal();

	int selected_id = 0;
	for (int i = 0; i < minfos.size(); i++) {
		m_SignalSelector->add_item(minfos[i].name);
		if (!signal.is_empty()) {
			if (minfos[i].name == signal) {
				signal.clear();
				selected_id = i;
			}
		}
	}

	m_SignalSelector->select(selected_id);
}

void EmitSignalNodeEditorProp::OnSignalSelected(int id) {
	Node *target_node = Object::cast_to<Node>(ObjectDB::get_instance(m_TargetNode));
	if (!target_node)
		return;

	List<MethodInfo> minfos;
	target_node->get_signal_list(&minfos);

	Array params;
	int first_defarg = minfos[id].arguments.size() - minfos[id].default_arguments.size();

	for (int i = 0; i < minfos[id].arguments.size(); i++) {
		if (i >= first_defarg) {
			Variant arg = minfos[id].default_arguments[i - first_defarg];
			params.push_back(arg);
		} else {
			Callable::CallError ce;
			Variant arg;
			Variant::construct(minfos[id].arguments[i].type, arg, nullptr, 0, ce);
			params.push_back(arg);
		}
	}

	UndoRedo *undo_redo = EditorNode::get_undo_redo();
	undo_redo->create_action(TTR("Add Signal"));

	undo_redo->add_do_method(*m_SignalNode, "set_signal_name", minfos[id].name);
	undo_redo->add_do_method(*m_SignalNode, "set_args", params);

	String old_name = m_SignalNode->GetCallbackSignal();
	if (old_name.is_empty())
		old_name = "<signal>";

	undo_redo->add_undo_method(*m_SignalNode, "set_signal_name", old_name);
	undo_redo->add_undo_method(*m_SignalNode, "set_args", m_SignalNode->GetArgs());

	undo_redo->commit_action();
}

void EmitSignalNodeEditorProp::OnPathChanged() {
	SetupNode(m_SignalNode);
	m_SignalSelector->clear();
	SetupOptions();
}

EmitSignalNodeEditorPlugin::EmitSignalNodeEditorPlugin() {
	add_inspector_plugin(memnew(EmitSignalInspectorPlugin));
}

} //namespace behaviour_tree::editor

#endif
