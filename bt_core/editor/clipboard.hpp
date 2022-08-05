#pragma once

#include <set>
#include <vector>

#include "core/object/undo_redo.h"
#include "scene/gui/graph_edit.h"
#include "../node_behaviour.hpp"


#if TOOLS_ENABLED
namespace behaviour_tree::editor
{
	class BehaviourTreeViewer;

	class ClipboardBuffer
	{
	public:
		struct Connection
		{
			String From;
			String To;

			Connection(const String& from, const String& to):
				From(from), To(to) {}
		};

		struct CopyItem
		{
			Ref<IBehaviourTreeNodeBehaviour> Node;
			Vector2 Position;
			String GraphId;
		};

	public:
		void SetView(BehaviourTreeViewer* tree_viewer)
		{
			m_Viewer = tree_viewer;
		}

		void ClearBuffer()
		{
			m_CopyBuffer.clear();
		}

		bool IsEmpty() const
		{
			return m_CopyBuffer.empty();
		}

		void DoCopy();
		void DoCut();

		void DoPaste(bool duplicate);

	private:
		void DeleteFromBuffer();
		void CopyToBuffer();

		BehaviourTreeViewer* m_Viewer;
		std::vector<CopyItem> m_CopyBuffer;;
		std::vector<Connection> m_ConnectionBuffer;
	};
}
#endif // TOOLS_ENABLED
