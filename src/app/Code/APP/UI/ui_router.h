#ifndef __UI_ROUTER_H
#define __UI_ROUTER_H

#include "lvgl.h"

typedef enum{
    UI_PAGE_MAIN  = 0,
    UI_PAGE_PW_VERIFY,
    UI_PAGE_PW_CHANGE,
    UI_PAGE_FP_ADD,
    UI_PAGE_FP_DELETE,
    UI_PAGE_CARD_ADD,
    UI_PAGE_CARD_DELETE,
}ui_page_id_t;

void ui_router_navigate(ui_page_id_t id, void* param);
void lv_router_return_event_cb(lv_event_t* e);

#endif