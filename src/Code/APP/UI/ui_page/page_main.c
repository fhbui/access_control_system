#include "lvgl.h"
#include "ui_router.h"
#include "ui_widgets.h"

typedef struct{
    lv_obj_t* scr;
    uint8_t init_flag;
}_page_main_handle_t;

typedef struct {
    const char * label;
    ui_page_id_t target_page;
} main_menu_item_t;

// 菜单配置表：增加新功能只需在这里加一行，无需改动布局逻辑
static const main_menu_item_t menu_config[] = {
    {"Enter PassWord",     UI_PAGE_PW_VERIFY},
    {"Change PassWord",    UI_PAGE_PW_CHANGE},
    {"Add Fingerprint",    UI_PAGE_FP_ADD},
    {"Delete Fingerprint", UI_PAGE_FP_DELETE},
    {"Add Card",           UI_PAGE_CARD_ADD},
    {"Delete Card",        UI_PAGE_CARD_DELETE}
};

static _page_main_handle_t handle = {
    .scr = NULL,
    .init_flag = 0,
};
static const uint8_t MENU_COUNT = sizeof(menu_config) / sizeof(main_menu_item_t);

/**
 * 菜单按钮点击事件回调
 */
static void menu_btn_event_cb(lv_event_t * e) {
    ui_page_id_t target = (ui_page_id_t)(uintptr_t)lv_event_get_user_data(e);
    ui_router_navigate(target, NULL);
}

static void page_main_del_event_cb(lv_event_t * e){
    handle.scr = NULL;
    handle.init_flag = 0;
}

/**
 * 主页面初始化函数
 */
void page_main_init(void){
    // 业务逻辑处理

    // UI设计
    if(!handle.init_flag){
        lv_obj_t * scr = lv_obj_create(NULL);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
        ui_widget_apply_screen_style(scr);
        lv_obj_add_event_cb(scr, page_main_del_event_cb, LV_EVENT_DELETE, NULL);

        lv_obj_t * list = lv_list_create(scr);
        lv_obj_set_size(list, 220, 300);
        lv_obj_center(list);
        lv_obj_set_style_bg_opa(list, 0, 0); // 背景透明，显示底层色彩
        lv_obj_set_style_border_width(list, 0, 0);

        for (int i = 0; i < MENU_COUNT; i++) {
            lv_obj_t * btn = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, menu_config[i].label);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_add_event_cb(btn, menu_btn_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)menu_config[i].target_page);
        }

        handle.init_flag = 1;
        handle.scr = scr;
    }
    // 页面加载
    lv_scr_load_anim(handle.scr, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, true);
}
