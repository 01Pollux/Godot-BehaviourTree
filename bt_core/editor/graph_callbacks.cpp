
#if TOOLS_ENABLED
#include "../resources.hpp"
#include "../visual_resources.hpp"

#include "editor.hpp"
#include "editor/editor_settings.h"
#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"

namespace behaviour_tree::editor {
void BehaviourTreeViewer::OnReloadBehaviourTree() {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't reload a behaviour tree while editting");
		return;
	}
	m_UndoRedo->clear_history();
	Ref<VBehaviourTreeResource> new_tree =
			ResourceLoader::load(
					ResourceLoader::path_remap(m_VisualTreeHolder->get_path()),
					m_VisualTreeHolder->get_class(),
					ResourceFormatLoader::CACHE_MODE_IGNORE);
	StartEditing(*new_tree);
}

void BehaviourTreeViewer::OnSaveBehaviourTree(bool visual) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't save a behaviour tree while editting");
		return;
	}

	String path = m_VisualTreeHolder->get_path();
	if (!visual)
		path = path.substr(0, path.rfind(".")) + ".btree";

	int flg = 0;
	if (EditorSettings::get_singleton()->get("filesystem/on_save/compress_binary_resources")) {
		flg |= ResourceSaver::FLAG_COMPRESS;
	}

	Error err = ResourceSaver::save(m_VisualTreeHolder, path, flg | ResourceSaver::FLAG_REPLACE_SUBRESOURCE_PATHS);
	ERR_FAIL_COND_MSG(err != Error::OK, String("Failed to save behaviour tree (message: ") + error_names[err]);
}

void BehaviourTreeViewer::OnGuiInput(const Ref<InputEvent> &input_event) {
	Ref<InputEventMouseButton> mouse_event = input_event;
	if (mouse_event.is_null() || !mouse_event->is_pressed() || mouse_event->get_button_index() != MouseButton::RIGHT)
		return;

	bool selecting_nodes = false;
	for (auto &node_gnode : m_GraphNodes) {
		GraphNode *gnode = node_gnode.second;
		if (gnode->is_selected()) {
			selecting_nodes = true;
			break;
		}
	}

	m_NodeSpawnPosition = m_Graph->get_local_mouse_position();
	if (!selecting_nodes && m_Clipboard.IsEmpty()) {
		DisplayMembersDialog();
	} else {
		m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_CUT, !selecting_nodes);
		m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_COPY, !selecting_nodes);
		m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_PASTE, m_Clipboard.IsEmpty());
		m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_DUPLICATE, !selecting_nodes);
		m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_CLEAR_BUFFER, m_Clipboard.IsEmpty());

		OpenItemsPopup();
	}
}

void BehaviourTreeViewer::OnNodeSelected(Object *object) {
	Ref<IBehaviourTreeNodeBehaviour> cur_node = object->get_meta("_node");
	if (cur_node.is_valid())
		EditorNode::get_singleton()->push_item(*cur_node, "", true);
}

void BehaviourTreeViewer::OpenItemsPopup() {
	Vector2 popup_position = get_local_mouse_position() + get_screen_position();

	// Keep dialog within window bounds.
	Rect2 window_rect = Rect2(
			DisplayServer::get_singleton()->window_get_position(),
			DisplayServer::get_singleton()->window_get_size());
	Rect2 dialog_rect = Rect2(popup_position, m_RightclickPopup->get_size());
	Point2 delta_size = (dialog_rect.get_end() - window_rect.get_end()).max({});
	popup_position -= delta_size;

	m_RightclickPopup->set_position(popup_position);
	m_RightclickPopup->popup();
}

void BehaviourTreeViewer::OnPopupItemSelect(int id) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	switch (id) {
		case RightClickPopupType::RCPT_ADD:
			DisplayMembersDialog();
			break;
		case RightClickPopupType::RCPT_CUT:
			m_Clipboard.DoCut();
			break;
		case RightClickPopupType::RCPT_COPY:
			m_Clipboard.DoCopy();
			break;
		case RightClickPopupType::RCPT_PASTE:
			m_Clipboard.DoPaste(false);
			break;
		case RightClickPopupType::RCPT_DUPLICATE:
			m_Clipboard.DoPaste(true);
			break;
		case RightClickPopupType::RCPT_DELETE:
			OnDeleteNodesRequest({});
			break;
		case RightClickPopupType::RCPT_CLEAR_BUFFER:
			m_Clipboard.ClearBuffer();
			break;
	}
}
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
