#pragma once

#if TOOLS_ENABLED
#include "editor/editor_plugin.h"

namespace behaviour_tree::editor {
class EmitSignalNodeEditorPlugin : public EditorPlugin {
	GDCLASS(EmitSignalNodeEditorPlugin, EditorPlugin);

public:
	EmitSignalNodeEditorPlugin();
};
} //namespace behaviour_tree::editor
#endif
