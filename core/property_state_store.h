/*************************************************************************/
/*  property_state_store.h                                               */
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
#ifndef PROPERTY_STATE_STORE_H
#define PROPERTY_STATE_STORE_H

#include "object.h"
#include "map.h"

class StateGroup : public Object {
  GDCLASS(StateGroup, Object);

private:
  String name;

protected:
  static void _bind_methods();

public:
  StateGroup(const String& name);
  ~StateGroup();

  String get_name();
};

class PropertyStateStore : public Object {
  GDCLASS(PropertyStateStore, Object);

protected:
  static void _bind_methods();
  static PropertyStateStore* singleton;
  Map<String, StateGroup*> store_groups;

public:
  static PropertyStateStore* get_singleton();

  StateGroup* get_state_group(const String& group_name);
  StateGroup* create_state_group(const String& group_name);
  void add_state_group(StateGroup* p_group);

  PropertyStateStore();
  ~PropertyStateStore();
};

#endif // PROPERTY_STATE_STORE_H
