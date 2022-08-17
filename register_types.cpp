/*************************************************************************/
/*  register_types.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "register_types.h"
#include "core/object/class_db.h"

#include "bt_core/tree.hpp"
#include "bt_core/visual_resources.hpp"

#if TOOLS_ENABLED
#include "bt_core/editor/editor.hpp"
#include "bt_core/editor/remote_tree.hpp"
#endif

using namespace godot;

void initialize_behaviour_tree_module(ModuleInitializationLevel p_level) {
	using namespace behaviour_tree;
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		BehaviourTree::register_types();
		BehaviourTreeResource::register_types();
#if TOOLS_ENABLED
		VBehaviourTreeResource::register_types();
		GDREGISTER_CLASS(BehaviourTreeRemoteTreeHolder);
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::add_by_type<editor::BehaviourTreeEditor>();
#endif // TOOLS_ENABLED
	}
}

void uninitialize_behaviour_tree_module(ModuleInitializationLevel p_level) {
	using namespace behaviour_tree;
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		BehaviourTreeResource::unregister_types();
#if TOOLS_ENABLED
		VBehaviourTreeResource::unregister_types();
#endif
	}
}
