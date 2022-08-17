
#if TOOLS_ENABLED
#include "editor.hpp"

#include "../tree.hpp"
#include "../visual_resources.hpp"
#include "remote_tree.hpp"

#include "editor/debugger/editor_debugger_inspector.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/editor_node.h"

namespace behaviour_tree::editor {
BehaviourTreeEditor::BehaviourTreeEditor() {
	m_TreeViewer = memnew(BehaviourTreeViewer);
	m_TreeViewer->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	m_TreeViewer->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	m_TreeOpenButton = EditorNode::get_singleton()->add_bottom_panel_item(
			TTR("Behaviour Tree"),
			m_TreeViewer);

	make_visible(false);

	ScriptEditorDebugger *debugger = EditorDebuggerNode::get_singleton()->get_default_debugger();
	debugger->connect("remote_object_updated", callable_mp(this, &BehaviourTreeEditor::OnRemoteObjectUpdated));
}

BehaviourTreeEditor::~BehaviourTreeEditor() {
	EditorNode::get_singleton()->remove_bottom_panel_item(m_TreeOpenButton);
	memdelete(m_TreeViewer);
}

void BehaviourTreeEditor::make_visible(bool visible) {
	if (visible) {
		EditorNode::get_singleton()->make_bottom_panel_item_visible(m_TreeViewer);
		m_TreeViewer->set_process(true);
	} else {
		if (m_TreeViewer->is_visible_in_tree())
			EditorNode::get_singleton()->hide_bottom_panel();
		m_TreeViewer->set_process(false);
	}
}

void BehaviourTreeEditor::edit(Object *p_object) {
	m_TreeViewer->SetAllowEdits(true);
	m_TreeViewer->StartEditing(Object::cast_to<VBehaviourTreeResource>(p_object));
}

bool BehaviourTreeEditor::handles(Object *p_object) const {
	return p_object->is_class("VBehaviourTreeResource");
}

void BehaviourTreeEditor::OnRemoteObjectUpdated(ObjectID p_id) {
	ScriptEditorDebugger *debugger = EditorDebuggerNode::get_singleton()->get_default_debugger();
	Object *cur_obj = debugger->get_remote_object(p_id);

	if (auto remote_obj = Object::cast_to<EditorDebuggerRemoteObject>(cur_obj)) {
		for (PropertyInfo &prop : remote_obj->prop_list) {
			if (prop.name != "BehaviourTreeRemoteTreeHolder")
				continue;

			Ref<VBehaviourTreeResource> target_obj = remote_obj->prop_values.get("vbehaviour_tree");

			if (target_obj.is_valid()) {
				m_TreeViewer->SetAllowEdits(false);
				m_TreeViewer->StartEditing(*target_obj);
				m_TreeViewer->RemoteStates = remote_obj->prop_values.get("remote_states");

				make_visible(true);
			}
			break;
		}
	}
}
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
