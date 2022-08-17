
#if TOOLS_ENABLED
#include "editor.hpp"
#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"

#include "../composite_node.hpp"
#include "../decorator_node.hpp"
#include "../visual_resources.hpp"

namespace behaviour_tree::editor {
static bool GetNodeInfoFromResource(
		VBehaviourTreeResource *resource,
		const IBehaviourTreeNodeBehaviour *node,
		VBehaviourTreeResource::VisualNodeInfo &node_info) {
	BehaviourTree *tree = resource->GetTree().ptr();
	auto &tree_nodes = tree->GetNodes();

	for (size_t i = 0; i < tree_nodes.size(); i++) {
		if (*tree_nodes[i] == node) {
			node_info = resource->GetNodeInfo(i);
			return true;
		}
	}
	return false;
}

void BehaviourTreeViewer::AddNodeByIndex(int node_index) {
	const auto &load_info = GetRegisteredNodeInfo(node_index);

	size_t last_id = m_VisualTreeHolder->GetTree()->GetNodes().size();

	Ref<IBehaviourTreeNodeBehaviour> node;
	if (load_info.Script.is_null())
		node = m_VisualTreeHolder->GetTree()->GDCreateNode(load_info.Type);
	else {
		node = m_VisualTreeHolder->GetTree()->GDCreateNode(load_info.Script->get_instance_base_type());
		node->set_script(load_info.Script);
	}

	if (!node.is_null()) {
		auto &node_info = m_VisualTreeHolder->AddNodeInfo();

		node_info.Title = load_info.Name;
		node_info.Comment = load_info.Description;
		SetNodePosition(last_id, m_NodeSpawnPosition);

		UpdateLayout(last_id);
	}
}

void BehaviourTreeViewer::RemoveNodeByIndex(int node_index) {
	m_VisualTreeHolder->GetTree()->GDRemoveNodeByIndex(node_index);
	m_VisualTreeHolder->RemoveNodeInfo(node_index);
	RemoveGraphNode(node_index);
}

void BehaviourTreeViewer::AddNode(Ref<IBehaviourTreeNodeBehaviour> node, Vector2 position, const String &title, const String &comment) {
	auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();
	int last_node_id = tree_nodes.size();

	tree_nodes.push_back(node);
	auto &node_info = m_VisualTreeHolder->AddNodeInfo();

	SetNodePosition(last_node_id, position);
	node_info.Title = title;
	node_info.Comment = comment;

	UpdateLayout(last_node_id);
}

void BehaviourTreeViewer::RemoveNode(Ref<IBehaviourTreeNodeBehaviour> node) {
	auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();

	for (size_t i = 0; i < tree_nodes.size(); i++) {
		if (*tree_nodes[i] == *node) {
			m_VisualTreeHolder->GetTree()->GDRemoveNodeByIndex(i);
			m_VisualTreeHolder->RemoveNodeInfo(i);
			break;
		}
	}
}

void BehaviourTreeViewer::SetNodeTitle(int node_index, const String &title) {
	auto &node_info = m_VisualTreeHolder->GetNodeInfo(node_index);
	node_info.Title = title;
}

void BehaviourTreeViewer::SetNodeComment(int node_index, const String &comment) {
	auto &node_info = m_VisualTreeHolder->GetNodeInfo(node_index);
	node_info.Comment = comment;
}

GraphNode *BehaviourTreeViewer::CreateGraphNode(int node_index) {
	BehaviourTree *tree = m_VisualTreeHolder->GetTree().ptr();
	auto &tree_nodes = tree->GetNodes();

	IBehaviourTreeNodeBehaviour *tree_node = tree_nodes[node_index].ptr();
	bool is_root = tree->GetRootNode() == tree_node;

	auto &node_info = m_VisualTreeHolder->GetNodeInfo(node_index);

	GraphNode *gnode = memnew(GraphNode);
	m_Graph->add_child(gnode);

	gnode->set_position_offset(node_info.Position * EDSCALE);
	gnode->set_title(node_info.Title);

	gnode->set_theme(is_root ? m_Themes["root"] : m_Themes["default"]);
	gnode->set_overlay(GraphNode::OVERLAY_BREAKPOINT);

	gnode->set_meta("_node", tree_node);
	m_GraphNodes[tree_node] = gnode;

	gnode->set_custom_minimum_size(Size2(195 * EDSCALE, 100 * EDSCALE));
	gnode->connect("dragged", callable_mp(this, &BehaviourTreeViewer::OnNodeDrag).bind(node_index));

	VBoxContainer *container = memnew(VBoxContainer);
	gnode->add_child(container);
	container->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	Button *set_root = memnew(Button);
	set_root->set_text(TTR("Set root"));
	container->add_child(set_root);
	set_root->connect("pressed", callable_mp(this, &BehaviourTreeViewer::SetAsRoot).bind(node_index));

	TextEdit *description = memnew(TextEdit);
	container->add_child(description);
	description->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	description->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	description->set_text(node_info.Comment);

	description->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_BOUNDARY);
	description->set_fit_content_height_enabled(true);
	description->set("theme_override_font_sizes", 14);
	description->connect("text_changed", callable_mp(this, &BehaviourTreeViewer::OnCommentChanged).bind(description, node_index));

	return gnode;
}

void BehaviourTreeViewer::LinkGraphNode(GraphNode *gnode, int node_index, const Color &port_color) {
	BehaviourTree *tree = m_VisualTreeHolder->GetTree().ptr();
	auto &tree_nodes = tree->GetNodes();
	IBehaviourTreeNodeBehaviour *tree_node = tree_nodes[node_index].ptr();

	IBehaviourTreeCompositeNode *composite = Object::cast_to<IBehaviourTreeCompositeNode>(tree_node);
	IBehaviourTreeDecoratorNode *decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(tree_node);

	// 0 : no link
	// 1 : our new node is the output
	// 2 : our new node is the input
	int link_type = 0;
	StringName this_node_id = gnode->get_name();

	if (composite || decorator) {
		gnode->set_slot(0, true, 0, port_color, true, 0, port_color);
		gnode->set_slot_color_left(0, Color(1.f, 1.f, 0.f, 0.65f));
		gnode->set_slot_color_right(0, Color(1.f, 0.6f, 0.4f, 1.f));

		std::vector<IBehaviourTreeNodeBehaviour *> childrens;
		tree_node->GetChildrens(childrens);

		for (IBehaviourTreeNodeBehaviour *child : childrens) {
			GraphNode *child_gnode = FindGraphNode(child);
			m_Graph->connect_node(this_node_id, 0, child_gnode->get_name(), 0);
		}

		if (CheckPendingLinkFromNode(tree_node))
			link_type = 1;
		else if (!m_PendingLinkToNode.is_empty()) {
			if (composite) {
				Node *node = m_Graph->get_node(m_PendingLinkToNode);
				IBehaviourTreeNodeBehaviour *to_node = node ? Object::cast_to<IBehaviourTreeNodeBehaviour>(node->get_meta("_node")) : nullptr;
				if (to_node) {
					composite->AddChild(to_node);
					link_type = 2;
				}
			} else if (decorator) {
				Node *node = m_Graph->get_node(m_PendingLinkToNode);
				IBehaviourTreeNodeBehaviour *to_node = node ? Object::cast_to<IBehaviourTreeNodeBehaviour>(node->get_meta("_node")) : nullptr;
				if (to_node) {
					decorator->SetChild(to_node);
					link_type = 2;
				}
			}
		}
	} else {
		gnode->set_slot(0, true, 0, port_color, false, 0, Color());
		gnode->set_slot_color_left(0, Color(1.f, 1.f, 0.f, 0.65f));

		if (CheckPendingLinkFromNode(tree_node))
			link_type = 1;
	}

	switch (link_type) {
		case 1: {
			m_Graph->connect_node(m_PendingLinkFromNode, 0, this_node_id, 0);
			break;
		}
		case 2: {
			m_Graph->connect_node(this_node_id, 0, m_PendingLinkToNode, 0);
			break;
		}
	}

	m_PendingLinkToNode.clear();
	m_PendingLinkFromNode.clear();
}

void BehaviourTreeViewer::OnNodeDrag(const Vector2 &p_from, const Vector2 &p_to, int node_index) {
	if (m_PendingDragNodes.empty())
		call_deferred(SNAME("_on_nodes_dragged"));
	m_PendingDragNodes.emplace_back(p_from, p_to, node_index);
}

void BehaviourTreeViewer::Deffered_OnNodeDrag() {
	auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();

	m_UndoRedo->create_action("Node(s) moved");
	for (auto &[p_from, p_to, node_index] : m_PendingDragNodes) {
		GraphNode *gnode = FindGraphNode(node_index);

		m_UndoRedo->add_do_method(this, "_set_node_position", node_index, p_to);
		m_UndoRedo->add_undo_method(this, "_set_node_position", node_index, p_from);

		m_UndoRedo->add_do_method(gnode, "set_position_offset", p_to);
		m_UndoRedo->add_undo_method(gnode, "set_position_offset", p_from);
	}
	m_PendingDragNodes.clear();
	m_UndoRedo->commit_action();
}

void BehaviourTreeViewer::SetNodePosition(int node_index, const Vector2 &position) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}
	auto &node_info = m_VisualTreeHolder->GetNodeInfo(node_index);
	node_info.Position = position;

	auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();

	IBehaviourTreeNodeBehaviour *tree_node = *tree_nodes[node_index];
	Ref<IBehaviourTreeCompositeNode> composite = *m_VisualTreeHolder->GetTree()->GetParentOfNode(tree_node);
	if (composite.is_null())
		return;

	auto &childrens = composite->GetChildrens();
	if (childrens.size() <= 1)
		return;

	struct SortData {
		Ref<IBehaviourTreeNodeBehaviour> Node;
		Vector2 Position;

		SortData(Ref<IBehaviourTreeNodeBehaviour> node, Vector2 position) :
				Node(node), Position(position) {}
	};

	auto compare_func = [](const SortData &lhs, const SortData &rhs) noexcept -> bool {
		if (lhs.Position.y >= rhs.Position.y)
			return lhs.Position.x < rhs.Position.x;
		return true;
	};

	std::set<SortData, decltype(compare_func)> sorted_childrens(compare_func);

	for (size_t i = 0; i < childrens.size(); i++) {
		for (size_t j = 0; j < tree_nodes.size(); j++) {
			if (childrens[i] == *tree_nodes[j]) {
				sorted_childrens.emplace(
						childrens[i],
						m_VisualTreeHolder->GetNodeInfo(j).Position);
				break;
			}
		}
	}

	composite->GetChildrens().clear();
	for (auto &data : sorted_childrens) {
		composite->GetChildrens().emplace_back(data.Node);
	}
}

void BehaviourTreeViewer::SetAsRoot(int node_index) {
	BehaviourTree *tree = m_VisualTreeHolder->GetTree().ptr();
	auto &tree_nodes = tree->GetNodes();

	Ref<IBehaviourTreeNodeBehaviour> new_root = tree->GetNodes()[node_index];
	if (tree->GetRootNode() != *new_root) {
		m_UndoRedo->create_action("Set Root");

		m_UndoRedo->add_do_method(tree, "set_root", new_root);
		m_UndoRedo->add_undo_method(tree, "set_root", tree->GDGetRootNode());

		GraphNode *new_gnode = FindGraphNode(node_index);
		m_UndoRedo->add_do_method(new_gnode, "set_theme", m_Themes["root"]);
		m_UndoRedo->add_undo_method(new_gnode, "set_theme", m_Themes["default"]);

		if (GraphNode *old_gnode = FindGraphNode(*tree->GDGetRootNode())) {
			m_UndoRedo->add_do_method(old_gnode, "set_theme", m_Themes["default"]);
			m_UndoRedo->add_undo_method(old_gnode, "set_theme", m_Themes["root"]);
		}

		m_UndoRedo->commit_action();
	}
}

void BehaviourTreeViewer::OnCommentChanged(TextEdit *text_edit, int node_index) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	auto &node_info = m_VisualTreeHolder->GetNodeInfo(node_index);
	node_info.Comment = text_edit->get_text();
}

void BehaviourTreeViewer::OnNodeConnect(const String &from, int, const String &to, int) {
	if (from == to)
		return;
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	BehaviourTree *tree = m_VisualTreeHolder->GetTree().ptr();
	auto &tree_nodes = tree->GetNodes();

	m_UndoRedo->create_action("Connect Node");

	m_UndoRedo->add_do_method(m_Graph, "connect_node", from, 0, to, 0);
	m_UndoRedo->add_undo_method(m_Graph, "disconnect_node", from, 0, to, 0);

	IBehaviourTreeNodeBehaviour *from_node = Object::cast_to<IBehaviourTreeNodeBehaviour>(m_Graph->get_node(from)->get_meta("_node"));
	IBehaviourTreeNodeBehaviour *to_node = Object::cast_to<IBehaviourTreeNodeBehaviour>(m_Graph->get_node(to)->get_meta("_node"));

	if (IBehaviourTreeCompositeNode *composite = Object::cast_to<IBehaviourTreeCompositeNode>(from_node)) {
		m_UndoRedo->add_do_method(composite, "add_btchild", Ref(to_node));
		m_UndoRedo->add_undo_method(composite, "remove_btchild", Ref(to_node));
	} else if (IBehaviourTreeDecoratorNode *decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(from_node)) {
		Ref<IBehaviourTreeNodeBehaviour> previous_child = decorator->GetChild();
		if (previous_child.is_valid()) {
			if (GraphNode *old_gnode = FindGraphNode(*previous_child)) {
				m_UndoRedo->add_do_method(m_Graph, "disconnect_node", from, 0, old_gnode->get_name(), 0);
				m_UndoRedo->add_undo_method(m_Graph, "connect_node", from, 0, old_gnode->get_name(), 0);
			}
		}

		m_UndoRedo->add_do_method(decorator, "set_btchild", Ref(to_node));
		m_UndoRedo->add_undo_method(decorator, "set_btchild", previous_child);
	}

	m_UndoRedo->commit_action();
}

void BehaviourTreeViewer::OnNodeDisconnect(const String &from, int, const String &to, int) {
	if (from == to)
		return;
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	BehaviourTree *tree = m_VisualTreeHolder->GetTree().ptr();
	auto &tree_nodes = tree->GetNodes();

	m_UndoRedo->create_action("Disconnect Node");

	m_UndoRedo->add_do_method(m_Graph, "disconnect_node", from, 0, to, 0);
	m_UndoRedo->add_undo_method(m_Graph, "connect_node", from, 0, to, 0);

	IBehaviourTreeNodeBehaviour *from_node = Object::cast_to<IBehaviourTreeNodeBehaviour>(m_Graph->get_node(from)->get_meta("_node"));
	IBehaviourTreeNodeBehaviour *to_node = Object::cast_to<IBehaviourTreeNodeBehaviour>(m_Graph->get_node(to)->get_meta("_node"));

	if (IBehaviourTreeCompositeNode *composite = Object::cast_to<IBehaviourTreeCompositeNode>(from_node)) {
		m_UndoRedo->add_do_method(composite, "remove_btchild", Ref(to_node));
		m_UndoRedo->add_undo_method(composite, "add_btchild", Ref(to_node));
	}

	if (IBehaviourTreeDecoratorNode *decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(to_node);
			decorator && decorator->GetChild() == to_node) {
		m_UndoRedo->add_do_method(decorator, "set_btchild", Ref<IBehaviourTreeNodeBehaviour>());
		m_UndoRedo->add_undo_method(decorator, "set_btchild", decorator->GetChild());
	}

	m_UndoRedo->commit_action();
}

void BehaviourTreeViewer::OnNodeConnectToEmpty(const String &from, int, const Vector2 &position) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	m_PendingLinkFromNode = from;
	DisplayMembersDialog();
	m_NodeSpawnPosition = position;
}

void BehaviourTreeViewer::OnNodeConnectFromEmpty(const String &to, int, const Vector2 &position) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	m_PendingLinkToNode = to;
	DisplayMembersDialog();
	m_NodeSpawnPosition = position;
}

bool BehaviourTreeViewer::CheckPendingLinkFromNode(IBehaviourTreeNodeBehaviour *to_node) {
	if (!m_PendingLinkFromNode.is_empty()) {
		Node *gnode = m_Graph->get_node(m_PendingLinkFromNode);
		if (!gnode)
			return false;
		Variant from_node = gnode->get_meta("_node");

		if (IBehaviourTreeCompositeNode *from_composite = Object::cast_to<IBehaviourTreeCompositeNode>(from_node)) {
			from_composite->AddChild(to_node);
			return true;
		} else if (IBehaviourTreeDecoratorNode *from_decorator = Object::cast_to<IBehaviourTreeDecoratorNode>(from_node)) {
			from_decorator->SetChild(to_node);
			return true;
		}
	}
	return false;
}

void BehaviourTreeViewer::OnDeleteNodesRequest(const TypedArray<StringName> &p_nodes) {
	if (!m_AllowEdits) {
		WARN_PRINT("Can't edit a behaviour tree while editting");
		return;
	}

	std::vector<IBehaviourTreeNodeBehaviour *> to_erase;

	if (p_nodes.is_empty()) {
		for (auto &node_gnode : m_GraphNodes) {
			GraphNode *gnode = node_gnode.second;
			if (gnode->is_selected())
				to_erase.push_back(node_gnode.first);
		}
	} else {
		auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();
		for (int i = 0; i < p_nodes.size(); i++) {
			IBehaviourTreeNodeBehaviour *node = Object::cast_to<IBehaviourTreeNodeBehaviour>(m_Graph->get_node(p_nodes[i])->get_meta("_node"));
			to_erase.push_back(node);
		}
	}

	if (to_erase.empty())
		return;

	m_UndoRedo->create_action(TTR("Delete Node(s)"));
	DeleteNodes(to_erase);
	m_UndoRedo->commit_action();
}

void BehaviourTreeViewer::DeleteNodes(const std::vector<IBehaviourTreeNodeBehaviour *> &to_erase_nodes) {
	List<GraphEdit::Connection> connections;
	m_Graph->get_connection_list(&connections);

	for (IBehaviourTreeNodeBehaviour *node : to_erase_nodes) {
		Ref ref_node = node;
		VBehaviourTreeResource::VisualNodeInfo node_info;
		GetNodeInfoFromResource(*m_VisualTreeHolder, node, node_info);

		m_UndoRedo->add_do_method(this, "_remove_node", ref_node);
		m_UndoRedo->add_undo_method(this, "_add_node", ref_node, node_info.Position, node_info.Title, node_info.Comment);

		Ref parent_node = m_VisualTreeHolder->GetTree()->GetParentOfNode(*ref_node);
		if (parent_node.is_valid()) {
			if (IBehaviourTreeCompositeNode *composite = Object::cast_to<IBehaviourTreeCompositeNode>(*parent_node))
				m_UndoRedo->add_undo_method(composite, "add_btchild", ref_node);
			else
				m_UndoRedo->add_undo_method(Object::cast_to<IBehaviourTreeDecoratorNode>(*parent_node), "set_btchild", ref_node);
		}
	}

	m_UndoRedo->add_do_method(this, "_update_graph_layout", -1);
	m_UndoRedo->add_undo_method(this, "_update_graph_layout", -1);
}
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
