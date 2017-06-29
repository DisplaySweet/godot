/*************************************************************************/
/*  property_state_store.cpp                                             */
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
#include "property_state_store.h"

PropertyStateStore* PropertyStateStore::singleton = NULL;

PropertyStateStore* PropertyStateStore::get_singleton() {
  return singleton;
}

StateGroup::StateGroup(const String& name) {
  this->name = name;
}

StateGroup::StateGroup() {
  name = "Untitled";
}

StateGroup::~StateGroup() {

}

void StateGroup::set_name(const String& name) {
  this->name = name;
}

String StateGroup::get_name() const {
  return name;
}

bool StateGroup::add_state(const String& state_name) {
  if (states.find(state_name) == NULL) {
    states[state_name] = Map<String, String>();
    return true;
  }
  return false;
}

void StateGroup::remove_state(const String& state_name) {
  Map<String, Map<String, String> >::Element* e = states.find(state_name);
  if (e != NULL) {
    states.erase(e);
  }
}

void StateGroup::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_name"), &StateGroup::get_name);
  ClassDB::bind_method(D_METHOD("add_state", "name"), &StateGroup::add_state);
  ClassDB::bind_method(D_METHOD("remove_state", "name"), &StateGroup::remove_state);
}

/*
  PropertyStateStore Methods below
*/

PropertyStateStore::PropertyStateStore() {
  singleton = this;
}

PropertyStateStore::~PropertyStateStore() {

}

StateGroup* PropertyStateStore::get_state_group(const String& group_name) {
  Map<String, StateGroup*>::Element* e = store_groups.find(group_name);
  if (e == NULL) {
    return NULL;
  }

  return e->get();
}

StateGroup* PropertyStateStore::create_state_group(const String& group_name) {
  StateGroup* temp = memnew(StateGroup(group_name));
  store_groups[group_name] = temp;
  return temp;
}

void PropertyStateStore::add_state_group(Object* p_group) {
  StateGroup* group = p_group->cast_to<StateGroup>();
  store_groups[group->get_name()] = group;
}

void PropertyStateStore::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_state_group", "name"), &PropertyStateStore::get_state_group);
  ClassDB::bind_method(D_METHOD("create_state_group", "name"), &PropertyStateStore::create_state_group);
  ClassDB::bind_method(D_METHOD("add_state_group", "group"), &PropertyStateStore::add_state_group);
}