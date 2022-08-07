#pragma once

#include <deque>
#include <map>
#include <vector>

#include "clipboard.hpp"
#include "editor/create_dialog.h"
#include "editor/plugins/script_editor_plugin.h"

#if TOOLS_ENABLED
namespace behaviour_tree {
class BehaviourTreeResource;
class VBehaviourTreeResource;
enum class NodeType;
} //namespace behaviour_tree

namespace behaviour_tree::editor {
enum RightClickPopupType {
	RCPT_ADD,
	RCPT_CUT,
	RCPT_COPY,
	RCPT_DUPLICATE,
	RCPT_PASTE,
	RCPT_DELETE,
	RCPT_CLEAR_BUFFER
};

struct QueryNodeInfo {
	String Name;
	String Description;
	String Category;
	String Type;
	Ref<Script> Script;

	QueryNodeInfo(
			const String &name,
			const String &category,
			const String &type,
			const String &description,
			const Ref<::Script> &script = nullptr) :
			Name(name),
			Description(description),
			Category(category),
			Type(type),
			Script(script) {}
};

class BehaviourTreeViewer : public VBoxContainer {
	GDCLASS(BehaviourTreeViewer, VBoxContainer);
	using QueryNodeInfoContainer = std::deque<QueryNodeInfo>;
	using QueryNodeInfoIterator = QueryNodeInfoContainer::const_iterator;

	friend class ClipboardBuffer;

public:
	static void _bind_methods();

	BehaviourTreeViewer();
	~BehaviourTreeViewer();

	void StartEditing(VBehaviourTreeResource *p_object);
	void UpdateLayout(int id);

private:
	void InitializeGraph();
	void InitializePopupMenu();
	void InitializeCreateNodesTree();
	void InitializeNodesInfo();
	void UpdateCustomNodesInfo();

private:
	void DisplayMembersDialog();
	void UpdateOptionsMenu();

private:
	void AddNodeByIndex(int index);
	void RemoveNodeByIndex(int index);

	void AddNode(Ref<IBehaviourTreeNodeBehaviour> node, Vector2 position, const String &title, const String &comment);
	void RemoveNode(Ref<IBehaviourTreeNodeBehaviour> node);

	void SetNodeTitle(int node_index, const String &title);
	void SetNodeComment(int node_index, const String &comment);

	GraphNode *CreateGraphNode(int node_idx, const Ref<Theme> &theme);
	void LinkGraphNode(GraphNode *graph_node, int node_idx, const Color &port_color);

	void OnNodeDrag(const Vector2 &p_from, const Vector2 &p_to, int node_idx);
	void Deffered_OnNodeDrag();

	void SetNodePosition(int node_idx, const Vector2 &position);
	void SetAsRoot(int node_idx);

	void OnNodeConnect(const String &from, int, const String &to, int);
	void OnNodeDisconnect(const String &from, int, const String &to, int);

	void OnNodeConnectToEmpty(const String &from, int, const Vector2 &position);
	void OnNodeConnectFromEmpty(const String &to, int, const Vector2 &position);

	bool CheckPendingLinkFromNode(IBehaviourTreeNodeBehaviour *to_node);

	void OnDeleteNodesRequest(const TypedArray<StringName> &p_nodes);
	void DeleteNodes(const std::vector<IBehaviourTreeNodeBehaviour *> &to_erase_nodes);

private:
	void OnReloadBehaviourTree();
	void OnSaveBehaviourTree(bool visual);

	void OnGuiInput(const Ref<InputEvent> &input_event);
	void OnNodeSelected(Object *node);

	void OpenItemsPopup();
	void OnPopupItemSelect(int id);

	void OnNodeTextFilterChange(const String &text);
	void OnNodeGUIInput(const Ref<InputEvent> &input_event, Tree *tree, LineEdit *line_edit);

	void OnToolItemPress(int index);

	void OnNodeMemberCreate();
	void OnNodeMemberCreateSelect();
	void OnNodeMemberCreateCancel();
	void Deffered_OnNodeMemberCreateCancel();

private:
	void Filter_CollectNodes(std::vector<size_t> &options);

	GraphNode *FindGraphNode(int node_index);
	GraphNode *FindGraphNode(IBehaviourTreeNodeBehaviour *node);
	void RemoveGraphNode(int node_index);

private:
	QueryNodeInfo &GetRegisteredNodeInfo(int index) noexcept {
		return index < m_RegisteredNodesInfo.size() ? m_RegisteredNodesInfo[index] : m_CustomNodesInfo[index - m_RegisteredNodesInfo.size()];
	}

private:
	Ref<VBehaviourTreeResource> m_VisualTreeHolder{};

	UndoRedo *m_UndoRedo{};
	GraphEdit *m_Graph{};
	PopupMenu *m_RightclickPopup{};

	LineEdit *m_NodesTextFilter{};
	MenuButton *m_NodesTools{};
	Tree *m_NodesTree{};
	RichTextLabel *m_NodeDescription{};
	ConfirmationDialog *m_NodeCreateDialog{};

	ClipboardBuffer m_Clipboard;
	QueryNodeInfoContainer m_RegisteredNodesInfo, m_CustomNodesInfo;
	Vector2 m_NodeSpawnPosition;

	std::vector<
			std::tuple<Vector2, Vector2, int>>
			m_PendingDragNodes;
	std::map<IBehaviourTreeNodeBehaviour *, GraphNode *> m_GraphNodes;

	String m_PendingLinkFromNode, m_PendingLinkToNode;
};

class BehaviourTreeEditor : public EditorPlugin {
	GDCLASS(BehaviourTreeEditor, EditorPlugin);

public:
	BehaviourTreeEditor();
	~BehaviourTreeEditor();

public:
	String get_name() const override { return "Behaviour Tree"; }
	//bool has_main_screen() const;
	void make_visible(bool p_visible) override;
	void edit(Object *p_object) override;
	bool handles(Object *p_object) const override;

private:
	BehaviourTreeViewer *m_TreeViewer;
	Button *m_TreeOpenButton;
};
} //namespace behaviour_tree::editor
#endif // TOOLS_ENABLED
