#include "../resources.hpp"
#include "../visual_resources.hpp"

#include "editor.hpp"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "editor/editor_file_dialog.h"

#if TOOLS_ENABLED
namespace behaviour_tree::editor
{
	void BehaviourTreeViewer::OnReloadBehaviourTree()
	{
		m_VisualTreeHolder->reload_from_file();
	}

	void BehaviourTreeViewer::OnSaveBehaviourTree(bool visual)
	{
		if (visual)
			EditorNode::get_singleton()->save_resource(m_VisualTreeHolder);
		else
		{
			String path = m_VisualTreeHolder->get_path();
			EditorNode::get_singleton()->save_resource_in_path(
				m_VisualTreeHolder->GetResources(),
				path.substr(0, path.rfind(".")) + ".btree"
			);
		}
	}

	
	void BehaviourTreeViewer::OnGuiInput(const Ref<InputEvent>& input_event)
	{
		Ref<InputEventMouseButton> mouse_event = input_event;
		if (mouse_event.is_null() || !mouse_event->is_pressed() || mouse_event->get_button_index() != MouseButton::RIGHT)
			return;

		bool selecting_nodes = false;
		for (auto& node_gnode : m_GraphNodes)
		{
			GraphNode* gnode = node_gnode.second;
			if (gnode->is_selected())
			{
				selecting_nodes = true;
				break;
			}
		}

		if (!selecting_nodes && m_Clipboard.IsEmpty())
		{
			m_NodeSpawnPosition = m_Graph->get_local_mouse_position();
			DisplayMembersDialog();
		}
		else
		{
			m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_CUT, !selecting_nodes);
			m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_COPY, !selecting_nodes);
			m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_PASTE, m_Clipboard.IsEmpty());
			m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_DUPLICATE, !selecting_nodes);
			m_RightclickPopup->set_item_disabled(RightClickPopupType::RCPT_CLEAR_BUFFER, m_Clipboard.IsEmpty());

			OpenItemsPopup();
		}
	}


	void BehaviourTreeViewer::OnNodeSelected(Object* object)
	{
		Ref<IBehaviourTreeNodeBehaviour> cur_node = object->get_meta("_node");
		if (cur_node.is_valid())
			EditorNode::get_singleton()->push_item(*cur_node, "Test Property", true);
	}


	void BehaviourTreeViewer::OpenItemsPopup()
	{
		Vector2 popup_position = get_local_mouse_position() + get_screen_position();

		// Keep dialog within window bounds.
		Rect2 window_rect = Rect2(
			DisplayServer::get_singleton()->window_get_position(),
			DisplayServer::get_singleton()->window_get_size()
		);
		Rect2 dialog_rect = Rect2(popup_position, m_RightclickPopup->get_size());
		Point2 delta_size = (dialog_rect.get_end() - window_rect.get_end()).max({});
		popup_position -= delta_size;

		m_RightclickPopup->set_position(popup_position);
		m_RightclickPopup->popup();

		m_NodeSpawnPosition = m_Graph->get_local_mouse_position();
	}


	void BehaviourTreeViewer::OnPopupItemSelect(int id)
	{
		switch (id)
		{
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
}
#endif // TOOLS_ENABLED
