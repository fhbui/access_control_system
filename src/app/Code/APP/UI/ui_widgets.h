#ifndef __UI_WIDGETS_H
#define __UI_WIDGETS_H

#include "lvgl.h"
#include "ui_router.h"

void ui_widget_apply_screen_style(lv_obj_t* scr);
lv_obj_t* ui_widget_create_back_btn(lv_obj_t* parent, ui_page_id_t target_page);
lv_obj_t* ui_widget_create_page_title(lv_obj_t* parent, const char* title);
lv_obj_t* ui_widget_create_toast(const char * msg, uint32_t timeout, lv_event_cb_t custom_cb);
#endif
