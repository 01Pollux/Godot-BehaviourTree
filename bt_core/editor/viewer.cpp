
#include "editor.hpp"
#include <algorithm>
#include <map>

#include "../resources.hpp"
#include "../tree.hpp"
#include "../visual_resources.hpp"

#include "editor/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"

#if TOOLS_ENABLED
namespace behaviour_tree::editor {
void BehaviourTreeViewer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_add_node_idx", "node_index"), &BehaviourTreeViewer::AddNodeByIndex);
	ClassDB::bind_method(D_METHOD("_remove_node_idx", "node_index"), &BehaviourTreeViewer::RemoveNodeByIndex);

	ClassDB::bind_method(D_METHOD("_add_node", "node", "position", "title", "comment"), &BehaviourTreeViewer::AddNode);
	ClassDB::bind_method(D_METHOD("_remove_node", "node"), &BehaviourTreeViewer::RemoveNode);

	ClassDB::bind_method(D_METHOD("_set_node_title", "node_index", "title"), &BehaviourTreeViewer::SetNodeTitle);
	ClassDB::bind_method(D_METHOD("_set_node_comment", "node_index", "comment"), &BehaviourTreeViewer::SetNodeComment);

	ClassDB::bind_method(D_METHOD("_update_graph_layout", "node_index"), &BehaviourTreeViewer::UpdateLayout);
	ClassDB::bind_method(D_METHOD("_on_cancel_create_node_member"), &BehaviourTreeViewer::Deffered_OnNodeMemberCreateCancel);

	ClassDB::bind_method(D_METHOD("_on_nodes_dragged"), &BehaviourTreeViewer::Deffered_OnNodeDrag);
	ClassDB::bind_method(D_METHOD("_set_node_position", "node_index", "position"), &BehaviourTreeViewer::SetNodePosition);
}

BehaviourTreeViewer::BehaviourTreeViewer() {
	InitializeGraph();
	InitializePopupMenu();
	InitializeCreateNodesTree();
	InitializeNodesInfo();

	m_UndoRedo = EditorNode::get_undo_redo();
	m_Clipboard.SetView(this);
}

BehaviourTreeViewer::~BehaviourTreeViewer() {
	m_UndoRedo->clear_history();
}

void BehaviourTreeViewer::StartEditing(VBehaviourTreeResource *p_object) {
	if (m_VisualTreeHolder.ptr() == p_object)
		return;

	if (m_VisualTreeHolder.is_valid())
		m_VisualTreeHolder->disconnect("_on_custom_nodes_path_changed", callable_mp(this, &BehaviourTreeViewer::UpdateCustomNodesInfo));

	m_VisualTreeHolder = Ref(p_object);
	m_VisualTreeHolder->connect("_on_custom_nodes_path_changed", callable_mp(this, &BehaviourTreeViewer::UpdateCustomNodesInfo));
	UpdateCustomNodesInfo();

	UpdateLayout(-1);
}

void BehaviourTreeViewer::UpdateLayout(int id) {
	if (id >= 0)
		RemoveGraphNode(id);
	else {
		m_Graph->clear_connections();
		for (auto &node_gnode : m_GraphNodes) {
			m_Graph->remove_child(node_gnode.second);
			memdelete(node_gnode.second);
		}
		m_GraphNodes.clear();
	}

	m_Graph->show();

	Ref<Theme> theme;
	theme.instantiate();
	Ref<Font> label_font = EditorNode::get_singleton()->get_editor_theme()->get_font("main_msdf", "EditorFonts");
	theme->set_font("font", "Label", label_font);
	theme->set_font("font", "LineEdit", label_font);
	theme->set_font("font", "Button", label_font);

	const Color mono_color = get_theme_color(SNAME("mono_color"), SNAME("Editor"));

	if (id >= 0) {
		GraphNode *gnode = CreateGraphNode(id, theme);
		LinkGraphNode(gnode, id, mono_color);
	} else {
		auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();
		std::vector<GraphNode *> gnodes;
		gnodes.reserve(tree_nodes.size());

		for (size_t i = 0; i < tree_nodes.size(); i++)
			gnodes.push_back(CreateGraphNode(i, theme));

		for (size_t i = 0; i < gnodes.size(); i++)
			LinkGraphNode(gnodes[i], i, mono_color);
	}
}

void BehaviourTreeViewer::InitializeGraph() {
	set_custom_minimum_size(Size2(0, 325) * EDSCALE);

	HBoxContainer *menu_container = memnew(HBoxContainer);
	menu_container->set_alignment(HBoxContainer::ALIGNMENT_END);
	menu_container->set_anchor(Side::SIDE_RIGHT, .98f, false);

	// Save button
	Button *save_tree = memnew(Button);
	menu_container->add_child(save_tree);
	save_tree->set_text(TTR("Save"));
	save_tree->connect("pressed", callable_mp(this, &BehaviourTreeViewer::OnSaveBehaviourTree).bind(true));

	// Reload button
	Button *reload_tree = memnew(Button);
	menu_container->add_child(reload_tree);
	reload_tree->set_text(TTR("Reload"));
	reload_tree->connect("pressed", callable_mp(this, &BehaviourTreeViewer::OnReloadBehaviourTree));

	// Covert to tree
	Button *compressed_tree = memnew(Button);
	menu_container->add_child(compressed_tree);
	compressed_tree->set_text(TTR("Save as Tree"));
	compressed_tree->connect("pressed", callable_mp(this, &BehaviourTreeViewer::OnSaveBehaviourTree).bind(false));

	m_Graph = memnew(GraphEdit);
	m_Graph->add_child(menu_container);
	add_child(m_Graph);

	float graph_minimap_opacity = EditorSettings::get_singleton()->get("editors/visual_editors/minimap_opacity");
	m_Graph->set_minimap_opacity(graph_minimap_opacity);
	float graph_lines_curvature = EditorSettings::get_singleton()->get("editors/visual_editors/lines_curvature");
	m_Graph->set_connection_lines_curvature(graph_lines_curvature);
	m_Graph->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	m_Graph->add_valid_right_disconnect_type(0);

	m_Graph->connect("connection_request", callable_mp(this, &BehaviourTreeViewer::OnNodeConnect));
	m_Graph->connect("disconnection_request", callable_mp(this, &BehaviourTreeViewer::OnNodeDisconnect));

	m_Graph->connect("connection_to_empty", callable_mp(this, &BehaviourTreeViewer::OnNodeConnectToEmpty));
	m_Graph->connect("connection_from_empty", callable_mp(this, &BehaviourTreeViewer::OnNodeConnectFromEmpty));

	m_Graph->connect("delete_nodes_request", callable_mp(this, &BehaviourTreeViewer::OnDeleteNodesRequest));
	m_Graph->connect("gui_input", callable_mp(this, &BehaviourTreeViewer::OnGuiInput));

	m_Graph->connect("node_selected", callable_mp(this, &BehaviourTreeViewer::OnNodeSelected));
}

void BehaviourTreeViewer::InitializePopupMenu() {
	m_RightclickPopup = memnew(PopupMenu);
	add_child(m_RightclickPopup);
	m_RightclickPopup->add_item(TTR("Add Node"), RightClickPopupType::RCPT_ADD);
	m_RightclickPopup->add_separator();
	m_RightclickPopup->add_item(TTR("Cut"), RightClickPopupType::RCPT_CUT);
	m_RightclickPopup->add_item(TTR("Copy"), RightClickPopupType::RCPT_COPY);
	m_RightclickPopup->add_item(TTR("Paste"), RightClickPopupType::RCPT_PASTE);
	m_RightclickPopup->add_item(TTR("Delete"), RightClickPopupType::RCPT_DELETE);
	m_RightclickPopup->add_item(TTR("Clear Copy Buffer"), RightClickPopupType::RCPT_CLEAR_BUFFER);
	m_RightclickPopup->connect("id_pressed", callable_mp(this, &BehaviourTreeViewer::OnPopupItemSelect));
}

void BehaviourTreeViewer::InitializeCreateNodesTree() {
	VBoxContainer *members_vb = memnew(VBoxContainer);
	members_vb->set_v_size_flags(SIZE_EXPAND_FILL);

	HBoxContainer *filter_hb = memnew(HBoxContainer);
	members_vb->add_child(filter_hb);

	m_NodesTextFilter = memnew(LineEdit);
	filter_hb->add_child(m_NodesTextFilter);
	m_NodesTextFilter->set_h_size_flags(SIZE_EXPAND_FILL);
	m_NodesTextFilter->set_placeholder(TTR("Search"));

	m_NodesTools = memnew(MenuButton);
	filter_hb->add_child(m_NodesTools);
	m_NodesTools->set_tooltip(TTR("Options"));
	m_NodesTools->get_popup()->connect("id_pressed", callable_mp(this, &BehaviourTreeViewer::OnToolItemPress));
	m_NodesTools->get_popup()->add_item(TTR("Expand All"), 0);
	m_NodesTools->get_popup()->add_item(TTR("Collapse All"), 1);

	m_NodesTree = memnew(Tree);
	members_vb->add_child(m_NodesTree);
	m_NodesTree->set_drag_forwarding(this);
	m_NodesTree->set_h_size_flags(SIZE_EXPAND_FILL);
	m_NodesTree->set_v_size_flags(SIZE_EXPAND_FILL);
	m_NodesTree->set_hide_root(true);
	m_NodesTree->set_allow_reselect(true);
	m_NodesTree->set_hide_folding(false);
	m_NodesTree->set_custom_minimum_size(Size2(180 * EDSCALE, 200 * EDSCALE));

	HBoxContainer *desc_hbox = memnew(HBoxContainer);
	members_vb->add_child(desc_hbox);

	Label *desc_label = memnew(Label);
	desc_hbox->add_child(desc_label);
	desc_label->set_text(TTR("Description:"));

	desc_hbox->add_spacer();

	m_NodeDescription = memnew(RichTextLabel);
	members_vb->add_child(m_NodeDescription);
	m_NodeDescription->set_h_size_flags(SIZE_EXPAND_FILL);
	m_NodeDescription->set_v_size_flags(SIZE_FILL);
	m_NodeDescription->set_custom_minimum_size(Size2(0, 70 * EDSCALE));

	m_NodeCreateDialog = memnew(ConfirmationDialog);
	add_child(m_NodeCreateDialog);
	m_NodeCreateDialog->set_title(TTR("Create Behaviour Tree Node"));
	m_NodeCreateDialog->set_exclusive(false);
	m_NodeCreateDialog->add_child(members_vb);
	m_NodeCreateDialog->set_ok_button_text(TTR("Create"));
	m_NodeCreateDialog->get_ok_button()->set_disabled(true);

	m_NodeCreateDialog->set_min_size(Size2i(350, 370));

	m_NodeCreateDialog->get_ok_button()->connect("pressed", callable_mp(this, &BehaviourTreeViewer::OnNodeMemberCreate));
	m_NodeCreateDialog->connect("cancelled", callable_mp(this, &BehaviourTreeViewer::OnNodeMemberCreateCancel));

	m_NodesTree->connect("item_activated", callable_mp(this, &BehaviourTreeViewer::OnNodeMemberCreate));
	m_NodesTree->connect("item_selected", callable_mp(this, &BehaviourTreeViewer::OnNodeMemberCreateSelect));

	m_NodesTextFilter->connect("text_changed", callable_mp(this, &BehaviourTreeViewer::OnNodeTextFilterChange));
	m_NodesTextFilter->connect("gui_input", callable_mp(this, &BehaviourTreeViewer::OnNodeGUIInput).bind(m_NodesTree, m_NodesTextFilter));
}

void BehaviourTreeViewer::DisplayMembersDialog() {
	UpdateOptionsMenu();

	Vector2 position = m_Graph->get_size();
	position.x *= .5f;
	m_NodeCreateDialog->set_position((m_Graph->get_screen_position() + position) * EDSCALE);
	m_NodeCreateDialog->popup();

	m_NodesTextFilter->call_deferred(SNAME("grab_focus")); // Still not visible.
	m_NodesTextFilter->select_all();
}

void BehaviourTreeViewer::UpdateOptionsMenu() {
	m_NodeDescription->set_text("");
	m_NodeCreateDialog->get_ok_button()->set_disabled(true);

	m_NodesTree->clear();

	bool using_filter = !m_NodesTextFilter->get_text().is_empty();
	bool first_item = true;

	std::vector<size_t> options;
	if (using_filter)
		Filter_CollectNodes(options);
	else {
		options.reserve(m_RegisteredNodesInfo.size() + m_CustomNodesInfo.size());
		for (size_t i = 0; i < m_RegisteredNodesInfo.size() + m_CustomNodesInfo.size(); i++)
			options.push_back(i);
	}

	std::map<String, TreeItem *> sections;
	TreeItem *root = m_NodesTree->create_item();

	Color tree_item_çolor = get_theme_color(SNAME("warning_color"), SNAME("Editor"));

	for (size_t node_index : options) {
		QueryNodeInfo &node_info = GetRegisteredNodeInfo(node_index);

		const String &cur_category = node_info.Category;
		auto section_iter = sections.find(cur_category);
		TreeItem *category;

		if (section_iter == sections.end()) {
			Vector<String> split_sections = cur_category.split("/");
			category = root;
			String path_temp = "";
			for (int i = 0; i < split_sections.size(); i++) {
				path_temp += split_sections[i];
				section_iter = sections.find(path_temp);

				if (section_iter == sections.end()) {
					category = m_NodesTree->create_item(category);
					category->set_selectable(0, false);
					category->set_collapsed(!using_filter);
					category->set_text(0, split_sections[i]);

					sections[path_temp] = category;
				} else
					category = section_iter->second;
			}
		} else {
			category = section_iter->second;
		}

		TreeItem *item = m_NodesTree->create_item(category);
		item->set_text(0, node_info.Name);
		item->set_custom_color(0, tree_item_çolor);

		if (using_filter && first_item) {
			first_item = false;
			item->select(0);
			m_NodeDescription->set_text("This is description");
		}

		item->set_meta("_idx", node_index);
	}
}

void BehaviourTreeViewer::Filter_CollectNodes(std::vector<size_t> &options) {
	String filter = m_NodesTextFilter->get_text().strip_edges();

	for (size_t i = 0; i < m_RegisteredNodesInfo.size(); i++) {
		if (m_RegisteredNodesInfo[i].Name.findn(filter) != -1)
			options.push_back(i);
	}

	for (size_t i = 0; i < m_CustomNodesInfo.size(); i++) {
		if (m_CustomNodesInfo[i].Name.findn(filter) != -1)
			options.push_back(m_RegisteredNodesInfo.size() + i);
	}

	std::sort(options.begin(), options.end(),
			[this](const size_t lhs_idx, const size_t rhs_idx) {
				auto &lhs = GetRegisteredNodeInfo(lhs_idx);
				auto &rhs = GetRegisteredNodeInfo(rhs_idx);

				return lhs.Category.count("/") > rhs.Category.count("/") ||
						(lhs.Category + "/" + lhs.Name).naturalnocasecmp_to(rhs.Category + "/" + rhs.Name) < 0;
			});
}

GraphNode *BehaviourTreeViewer::FindGraphNode(int node_index) {
	auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();
	return FindGraphNode(*tree_nodes[node_index]);
}

GraphNode *BehaviourTreeViewer::FindGraphNode(IBehaviourTreeNodeBehaviour *node) {
	auto iter = m_GraphNodes.find(node);
	return iter == m_GraphNodes.end() ? nullptr : iter->second;
}

void BehaviourTreeViewer::RemoveGraphNode(int node_index) {
	auto &tree_nodes = m_VisualTreeHolder->GetTree()->GetNodes();
	auto iter = m_GraphNodes.find(*tree_nodes[node_index]);
	if (iter != m_GraphNodes.end()) {
		memdelete(iter->second);
		m_GraphNodes.erase(iter);
	}
}
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
