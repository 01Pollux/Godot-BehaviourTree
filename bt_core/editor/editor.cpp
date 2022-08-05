
#include "editor.hpp"

#include "../tree.hpp"
#include "../visual_resources.hpp"

#include "editor/editor_node.h"

#if TOOLS_ENABLED
namespace behaviour_tree::editor
{
	BehaviourTreeEditor::BehaviourTreeEditor()
	{
		m_TreeViewer = memnew(BehaviourTreeViewer);
		m_TreeOpenButton = EditorNode::get_singleton()->add_bottom_panel_item(
			TTR("Behaviour Tree"),
			m_TreeViewer
		);

		m_TreeOpenButton->hide();
	}

	BehaviourTreeEditor::~BehaviourTreeEditor()
	{
		EditorNode::get_singleton()->remove_bottom_panel_item(m_TreeOpenButton);
		memdelete(m_TreeViewer);
	}

	void BehaviourTreeEditor::make_visible(bool visible)
	{
		if (visible)
		{
			m_TreeOpenButton->show();
			EditorNode::get_singleton()->make_bottom_panel_item_visible(m_TreeViewer);
			m_TreeOpenButton->set_process(true);
		}
		else
		{
			m_TreeOpenButton->hide();
			if (m_TreeViewer->is_visible_in_tree())
				EditorNode::get_singleton()->hide_bottom_panel();
			m_TreeViewer->set_process(false);
		}
	}

	void BehaviourTreeEditor::edit(Object* p_object)
	{
		m_TreeViewer->StartEditing(Object::cast_to<VBehaviourTreeResource>(p_object));
	}

	bool BehaviourTreeEditor::handles(Object* p_object) const
	{
		return p_object->is_class("VBehaviourTreeResource");
	}
}
#endif // TOOLS_ENABLED
