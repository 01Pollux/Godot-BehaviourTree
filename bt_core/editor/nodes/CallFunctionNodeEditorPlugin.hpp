#pragma once

#if TOOLS_ENABLED
#include "editor/editor_plugin.h"

namespace behaviour_tree::editor {
class CallFunctionNodeEditorPlugin : public EditorPlugin {
	GDCLASS(CallFunctionNodeEditorPlugin, EditorPlugin);

public:
	CallFunctionNodeEditorPlugin();
};
} //namespace behaviour_tree::editor
#endif
