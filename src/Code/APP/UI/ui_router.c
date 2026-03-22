#include "lvgl.h"
#include "ui_router.h"

void page_main_init(void);
void page_password_init(int state);
void page_finger_init(int state);
void page_card_init(int state);

/**
 * 页面切换路由函数
 */
void ui_router_navigate(ui_page_id_t id, void* param){
    switch(id){
        case UI_PAGE_MAIN:
            page_main_init();
            break;
        case UI_PAGE_PW_VERIFY:
            page_password_init(0);
            break;
        case UI_PAGE_PW_CHANGE:
            page_password_init(1); 
            break;
        case UI_PAGE_FP_ADD:
            page_finger_init(0);
            break;
        case UI_PAGE_FP_DELETE:
            page_finger_init(1);
            break;
        case UI_PAGE_CARD_ADD:
            page_card_init(0);
            break;
        case UI_PAGE_CARD_DELETE:
            page_card_init(1);
            break;
    }
}

void lv_router_return_event_cb(lv_event_t* e){
    ui_router_navigate(UI_PAGE_MAIN, NULL);
}