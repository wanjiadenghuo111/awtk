﻿/**
 * File:   keyboard.h
 * Author: AWTK Develop Team
 * Brief:  keyboard
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-01-13 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/mem.h"
#include "base/keys.h"
#include "base/utils.h"
#include "base/enums.h"
#include "base/pages.h"
#include "base/locale.h"
#include "base/keyboard.h"
#include "base/prop_names.h"
#include "base/input_method.h"
#include "base/window_manager.h"

static ret_t keyboard_on_load(void* ctx, event_t* e);
static ret_t keyboard_on_action_info(void* ctx, event_t* e);

static ret_t keyboard_on_paint_self(widget_t* widget, canvas_t* c) {
  return widget_paint_helper(widget, c, NULL, NULL);
}

static ret_t keyboard_get_prop(widget_t* widget, const char* name, value_t* v) {
  keyboard_t* keyboard = KEYBOARD(widget);
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_OPEN_ANIM_HINT)) {
    value_set_str(v, keyboard->open_anim_hint.str);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_CLOSE_ANIM_HINT)) {
    value_set_str(v, keyboard->close_anim_hint.str);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t keyboard_set_prop(widget_t* widget, const char* name, const value_t* v) {
  keyboard_t* keyboard = KEYBOARD(widget);
  return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (tk_str_eq(name, WIDGET_PROP_ANIM_HINT)) {
    str_from_value(&(keyboard->open_anim_hint), v);
    str_from_value(&(keyboard->close_anim_hint), v);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_OPEN_ANIM_HINT)) {
    str_from_value(&(keyboard->open_anim_hint), v);
    return RET_OK;
  } else if (tk_str_eq(name, WIDGET_PROP_CLOSE_ANIM_HINT)) {
    str_from_value(&(keyboard->close_anim_hint), v);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t keyboard_destroy_default(widget_t* widget) {
  keyboard_t* keyboard = KEYBOARD(widget);
  input_method_off(input_method(), keyboard->action_info_id);

  return RET_OK;
}

static const widget_vtable_t s_keyboard_vtable = {.on_paint_self = keyboard_on_paint_self,
                                                  .set_prop = keyboard_set_prop,
                                                  .get_prop = keyboard_get_prop,
                                                  .destroy = keyboard_destroy_default};

widget_t* keyboard_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = NULL;
  keyboard_t* keyboard = TKMEM_ZALLOC(keyboard_t);
  return_value_if_fail(keyboard != NULL, NULL);

  widget = WIDGET(keyboard);
  widget_init(widget, NULL, WIDGET_KEYBOARD);
  widget->vt = &s_keyboard_vtable;
  array_init(&(keyboard->action_buttons), 0);

  if (parent == NULL) {
    parent = window_manager();
  }

  widget_move_resize(widget, x, y, w, h);
  return_value_if_fail(window_manager_add_child(parent, widget) == RET_OK, NULL);

  widget_update_style(widget);
  widget_on(widget, EVT_WINDOW_LOAD, keyboard_on_load, widget);
  keyboard->action_info_id =
      input_method_on(input_method(), EVT_IM_ACTION_INFO, keyboard_on_action_info, widget);

  return widget;
}

static ret_t keyboard_set_active_page(widget_t* button, const char* name) {
  widget_t* parent = button;
  while (parent != NULL && parent->type != WIDGET_PAGES) parent = parent->parent;

  return_value_if_fail(parent != NULL, RET_FAIL);

  return pages_set_active_by_name(parent, name);
}

#define STR_RETURN "return"
#define STR_ACTION "action"
#define STR_KEY_SPACE "space"
#define STR_KEY_PREFIX "key:"
#define STR_PAGE_PREFIX "page:"
#define STR_KEY_BACKSPACE "backspace"

static ret_t keyboard_on_button_click(void* ctx, event_t* e) {
  uint32_t code = 0;
  input_method_t* im = input_method();
  widget_t* button = WIDGET(e->target);
  const char* name = button->name.str;
  const char* key = strstr(name, STR_KEY_PREFIX);
  const char* page_name = strstr(name, STR_PAGE_PREFIX);

  if (page_name != NULL) {
    return keyboard_set_active_page(button, page_name + strlen(STR_PAGE_PREFIX));
  } else if (key != NULL) {
    key += strlen(STR_KEY_PREFIX);
    if (tk_str_eq(key, STR_KEY_BACKSPACE)) {
      code = FKEY_BACKSPACE;
    } else if (tk_str_eq(key, STR_KEY_SPACE)) {
      code = FKEY_SPACE;
    } else {
      code = *key;
    }

    return input_method_dispatch_key(im, code);
  } else {
    if (tk_str_eq(name, STR_ACTION)) {
      return input_method_dispatch_action(im);
    } else if (tk_str_eq(name, STR_KEY_SPACE)) {
      name = " ";
    }

    return input_method_commit_text(input_method(), name);
  }
}

static ret_t keyboard_update_action_buton_info(widget_t* button, const char* text, bool_t enable) {
  text = locale_tr(locale(), ((text && *text) ? text : STR_RETURN));

  widget_set_text_utf8(button, text);
  widget_set_enable(button, enable);

  return RET_OK;
}

static ret_t keyboard_on_action_info(void* ctx, event_t* e) {
  uint32_t i = 0;
  input_method_t* im = input_method();
  keyboard_t* keyboard = KEYBOARD(ctx);
  uint32_t nr = keyboard->action_buttons.size;
  widget_t** buttons = (widget_t**)keyboard->action_buttons.elms;
  im_action_button_info_event_t* info = (im_action_button_info_event_t*)e;

  for (i = 0; i < nr; i++) {
    keyboard_update_action_buton_info(buttons[i], im->action_buton_text, im->action_button_enable);
  }

  return RET_OK;
}

static ret_t keyboard_hook_buttons(void* ctx, void* iter) {
  widget_t* widget = WIDGET(iter);
  input_method_t* im = input_method();
  keyboard_t* keyboard = KEYBOARD(ctx);
  const char* name = widget->name.str;

  if (widget->type == WIDGET_BUTTON && name != NULL) {
    widget_on(widget, EVT_CLICK, keyboard_on_button_click, keyboard);

    if (tk_str_eq(name, STR_ACTION)) {
      array_push(&(keyboard->action_buttons), widget);
      keyboard_update_action_buton_info(widget, im->action_buton_text, im->action_button_enable);
    }
  }

  return RET_OK;
}

static ret_t keyboard_on_load(void* ctx, event_t* e) {
  widget_t* widget = WIDGET(ctx);

  (void)e;
  widget_foreach(widget, keyboard_hook_buttons, widget);

  return RET_OK;
}

ret_t keyboard_close(widget_t* widget) {
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  return window_manager_remove_child(widget->parent, widget);
}
