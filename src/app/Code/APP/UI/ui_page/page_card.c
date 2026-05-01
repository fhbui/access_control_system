#include "lvgl.h"
#include "ui_router.h"
#include "ui_service.h"

void page_card_init(int status){

    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    ui_widget_apply_screen_style(scr);
    ui_widget_create_back_btn(scr, UI_PAGE_MAIN);

    // 设置工作流
    if(status == 0){
        // 发布设置模块状态
        ui_widget_create_page_title(scr, "Add Card Page");
        ui_service_require(UI_SERVICE_SET_CARD_ADD, NULL);
    }
    else if(status == 1){
        // 发布设置模块状态
        ui_widget_create_page_title(scr, "Delete Card Page");
        ui_service_require(UI_SERVICE_SET_CARD_DEL, NULL);
    }

    // 页面加载
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);     // 不销毁主页状态
}
