/*************************************************************************/
/*  code_editor.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
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
#include "states_editor.h"

#include "editor/editor_settings.h"

#include "scene/gui/label.h"

void StatesEditor::_group_option(int p_op) {
  switch (p_op) {
  case NEW_STATE_GROUP:
    stateList->add_item("New option");
    print_line("New option added");
    break;
  case REMOVE_STATE_GROUP:
    stateList->remove_item(0);
    break;
  case ADD_STATE_TO_GROUP:
    break;
  case REMOVE_STATE_FROM_GROUP:
    break;

  default:
    break;
  }
}

StatesEditor::StatesEditor() {
  VBoxContainer* vlayout = memnew(VBoxContainer);
  add_child(vlayout);
  vlayout->set_v_size_flags(SIZE_EXPAND_FILL);

  HBoxContainer* hlayout1 = memnew(HBoxContainer);
  vlayout->add_child(hlayout1);

  state_menu = memnew(MenuButton);
  hlayout1->add_child(state_menu);
  state_menu->set_text(TTR("States"));
  state_menu->get_popup()->add_shortcut(ED_SHORTCUT("state_editor/new_group", TTR("New Group")), NEW_STATE_GROUP);
  state_menu->get_popup()->add_shortcut(ED_SHORTCUT("state_editor/remove_group", TTR("Remove Group")), REMOVE_STATE_GROUP);
  state_menu->get_popup()->add_shortcut(ED_SHORTCUT("state_editor/add_state_to_group", TTR("Add State")), ADD_STATE_TO_GROUP);
  state_menu->get_popup()->add_shortcut(ED_SHORTCUT("state_editor/remove_state_from_group", TTR("Remove State")), REMOVE_STATE_FROM_GROUP);
  state_menu->set_custom_minimum_size(Size2(50, 20));
  state_menu->get_popup()->connect("id_pressed", this, "_group_option");

  HBoxContainer* hlayout2 = memnew(HBoxContainer);
  vlayout->add_child(hlayout2);

  hlayout2->set_v_size_flags(SIZE_EXPAND_FILL);

  stateList = memnew(ItemList);
  hlayout2->add_child(stateList);
  stateList->set_custom_minimum_size(Size2(200, 200));

  stateList->add_item("Test");

  hlayout2->add_child(memnew(Label("This is a test")));

  ED_SHORTCUT("editor/editor_states", TTR("State Editor"));
}

void StatesEditor::_bind_methods() {
  ClassDB::bind_method("_group_option", &StatesEditor::_group_option);
}
