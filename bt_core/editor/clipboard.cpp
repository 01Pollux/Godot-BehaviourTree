
#if TOOLS_ENABLED
#include "clipboard.hpp"
#include "editor.hpp"

#include "editor/editor_scale.h"
#include "../composite_node.hpp"
#include "../decorator_node.hpp"
#include "../visual_resources.hpp"

namespace behaviour_tree::editor {
static bool GetNodeInfoFromResource(
		VisualBehaviourTree *resource,
		const IBehaviourTreeNodeBehaviour *node,
		VisualBehaviourTree::VisualNodeInfo &node_info) {
	auto &tree_nodes = resource->GetNodes();

	for (size_t i = 0; i < tree_nodes.size(); i++) {
		if (*tree_nodes[i] == node) {
			node_info = resource->GetNodeInfo(i);
			return true;
		}
	}
	return false;
}

void ClipboardBuffer::DoCopy() {
	m_Viewer->m_UndoRedo->create_action(TTR("Copy Node(s)"));
	CopyToBuffer();
	m_Viewer->m_UndoRedo->commit_action();
}

void ClipboardBuffer::DoCut() {
	m_Viewer->m_UndoRedo->create_action(TTR("Cut Node(s)"));

	CopyToBuffer();
	DeleteFromBuffer();

	m_Viewer->m_UndoRedo->commit_action();
}

void ClipboardBuffer::DoPaste(bool duplicate) {
	if (duplicate)
		CopyToBuffer();

	if (m_CopyBuffer.empty())
		return;
	UndoRedo *undo_redo = m_Viewer->m_UndoRedo;
	if (duplicate) {
		undo_redo->create_action(TTR("Duplicate Node(s)"));
	} else {
		if (m_CopyBuffer.empty())
			return;

		undo_redo->create_action(TTR("Paste Node(s)"));
	}

	float scale = m_Viewer->m_Graph->get_zoom();
	Vector2 mpos = m_Viewer->m_Graph->get_local_mouse_position();

	int cur_id = m_Viewer->m_VisualTreeHolder->GetNodes().size();
	std::map<IBehaviourTreeNodeBehaviour *, Ref<IBehaviourTreeNodeBehaviour>> added_nodes;

	for (CopyItem &item : m_CopyBuffer) {
		Ref node = item.Node->duplicate(true);
		added_nodes[*item.Node] = node;

		VisualBehaviourTree::VisualNodeInfo node_info;
		GetNodeInfoFromResource(*m_Viewer->m_VisualTreeHolder, *item.Node, node_info);

		undo_redo->add_do_method(m_Viewer, "_add_node", node, node_info.Position + Vector2(100, -70) * EDSCALE, node_info.Title, node_info.Comment);
		undo_redo->add_undo_method(m_Viewer, "_remove_node", node);
	}

	auto connect_to_child = [undo_redo](Ref<IBehaviourTreeNodeBehaviour> &parent, Ref<IBehaviourTreeNodeBehaviour> &child) {
		if (IBehaviourTreeCompositeNode *composite = Object::cast_to<IBehaviourTreeCompositeNode>(*parent)) {
			undo_redo->add_do_method(composite, "add_btchild", child);
			undo_redo->add_undo_method(composite, "remove_btchild", child);
		} else if (IBehaviourTreeDecoratorNode *decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(*parent)) {
			undo_redo->add_do_method(decorator, "set_btchild", child);
			undo_redo->add_undo_method(decorator, "set_btchild", decorator->GetChild());
		}
	};

	std::vector<IBehaviourTreeNodeBehaviour *> childrens;
	for (auto &[previous_node, new_node] : added_nodes) {
		if (!previous_node->GetChildrens(childrens))
			continue;

		// Find if our previous node have a child that exists in added_nodes
		for (auto &[nested_previous_node, nested_new_node] : added_nodes) {
			for (IBehaviourTreeNodeBehaviour *child : childrens) {
				if (child == nested_previous_node) {
					connect_to_child(new_node, nested_new_node);
					break;
				}
			}
		}

		childrens.clear();
	}

	undo_redo->add_do_method(m_Viewer, "_update_graph_layout", -1);
	undo_redo->add_undo_method(m_Viewer, "_update_graph_layout", -1);

	undo_redo->commit_action();
}

void ClipboardBuffer::DeleteFromBuffer() {
	UndoRedo *undo_redo = m_Viewer->m_UndoRedo;

	{
		using GraphConnection = GraphEdit::Connection;

		List<GraphConnection> connections;
		m_Viewer->m_Graph->get_connection_list(&connections);
		std::vector<GraphConnection *> used_connections;

		for (const CopyItem &item : m_CopyBuffer) {
			for (auto con_iter = connections.front(); con_iter; con_iter = con_iter->next()) {
				auto &connection = con_iter->get();
				if (connection.from != item.GraphId && connection.to != item.GraphId)
					continue;

				undo_redo->add_do_method(m_Viewer->m_Graph, "disconnect_node", connection.from, 0, connection.to, 0);

				bool already_exists = false;
				for (GraphConnection *used_con : used_connections) {
					if (used_con == &connection) {
						already_exists = true;
						break;
					}
				}

				if (already_exists)
					continue;

				undo_redo->add_undo_method(m_Viewer->m_Graph, "connect_node", connection.from, 0, connection.to, 0);
				used_connections.push_back(&connection);
			}
		}
	}

	for (const CopyItem &item : m_CopyBuffer) {
		VisualBehaviourTree::VisualNodeInfo node_info;
		GetNodeInfoFromResource(*m_Viewer->m_VisualTreeHolder, *item.Node, node_info);

		undo_redo->add_do_method(m_Viewer, "_remove_node", item.Node);
		undo_redo->add_undo_method(m_Viewer, "_add_node", item.Node, node_info.Position, node_info.Title, node_info.Comment);
	}

	undo_redo->add_do_method(m_Viewer, "_update_graph_layout", -1);
	undo_redo->add_undo_method(m_Viewer, "_update_graph_layout", -1);
}

void ClipboardBuffer::CopyToBuffer() {
	ClearBuffer();

	std::set<String> gnode_names;
	std::map<IBehaviourTreeNodeBehaviour *, Ref<IBehaviourTreeNodeBehaviour>> added_nodes;

	for (int i = 0; i < m_Viewer->m_Graph->get_child_count(); i++) {
		GraphNode *gnode = Object::cast_to<GraphNode>(m_Viewer->m_Graph->get_child(i));
		if (!gnode || !gnode->is_selected())
			continue;

		Ref node = Object::cast_to<IBehaviourTreeNodeBehaviour>(gnode->get_meta("_node"));
		if (node.is_null())
			continue;

		CopyItem &item = m_CopyBuffer.emplace_back();
		item.Node = node;
		item.Position = gnode->get_position_offset();
		item.GraphId = gnode->get_name();

		gnode_names.insert(gnode->get_name());
	}
}
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
