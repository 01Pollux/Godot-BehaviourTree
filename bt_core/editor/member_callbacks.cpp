
#include "editor.hpp"

#if TOOLS_ENABLED
namespace behaviour_tree::editor
{
	void BehaviourTreeViewer::OnNodeTextFilterChange(const String& text)
	{
		UpdateOptionsMenu();
	}

	void BehaviourTreeViewer::OnNodeGUIInput(const Ref<InputEvent>& input_event, Tree* tree, LineEdit* line_edit)
	{
		Ref<InputEventKey> key_event = input_event;
		if (key_event.is_valid() &&
			(key_event->get_keycode() == Key::UP ||
				key_event->get_keycode() == Key::DOWN ||
				key_event->get_keycode() == Key::ENTER ||
				key_event->get_keycode() == Key::KP_ENTER))
		{
			tree->gui_input(key_event);
			line_edit->accept_event();
		}
	}

	void BehaviourTreeViewer::OnToolItemPress(int index)
	{
		TreeItem* category = m_NodesTree->get_root()->get_first_child();

		switch (index)
		{
			//Expand All
		case 0:
		{
			while (category)
			{
				category->set_collapsed(false);
				TreeItem* sub_category = category->get_first_child();
				while (sub_category)
				{
					sub_category->set_collapsed(false);
					sub_category = sub_category->get_next();
				}
				category = category->get_next();
			}
			break;
		}

		//Collapse All
		case 1:
		{
			while (category)
			{
				category->set_collapsed(true);
				TreeItem* sub_category = category->get_first_child();
				while (sub_category)
				{
					sub_category->set_collapsed(true);
					sub_category = sub_category->get_next();
				}
				category = category->get_next();
			}
			break;
		}
		}
	}

	void BehaviourTreeViewer::OnNodeMemberCreate()
	{
		TreeItem* item = m_NodesTree->get_selected();
		if (item && item->has_meta("_idx"))
		{
			int node_index = item->get_meta("_idx");
			m_NodeCreateDialog->hide();
			m_UndoRedo->create_action("Add Node");

			m_UndoRedo->add_do_method(this, "_add_node_idx", node_index);
			m_UndoRedo->add_undo_method(this, "_remove_node_idx", node_index);

			m_UndoRedo->commit_action();
		}
	}

	void BehaviourTreeViewer::OnNodeMemberCreateSelect()
	{
		TreeItem* item = m_NodesTree->get_selected();
		if (item && item->has_meta("_idx"))
		{
			size_t index = item->get_meta("_idx");
			auto& node_info = GetRegisteredNodeInfo(index);

			m_NodeCreateDialog->get_ok_button()->set_disabled(false);
			m_NodeDescription->set_text(node_info.Description);
		}

		m_NodeDescription->set_text("");
		m_NodeCreateDialog->get_ok_button()->set_disabled(true);
	}

	void BehaviourTreeViewer::OnNodeMemberCreateCancel()
	{
		call_deferred("_on_cancel_node_member");
	}

	void BehaviourTreeViewer::Deffered_OnNodeMemberCreateCancel()
	{
		m_PendingLinkToNode.clear();
		m_PendingLinkFromNode.clear();
	}
}
#endif // TOOLS_ENABLED
