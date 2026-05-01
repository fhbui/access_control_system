/* 一些共用的控件创建函数 */
#include "lvgl.h"
#include "ui_widgets.h"

void ui_widget_apply_screen_style(lv_obj_t* scr){
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色
}

void ui_widget_apply_menu_btn_style(lv_obj_t* btn){
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);
}

// 返回按键控件
static void back_btn_event_cb(lv_event_t * e) {
    ui_page_id_t target = (ui_page_id_t)(uintptr_t)lv_event_get_user_data(e);
    ui_router_navigate(target, NULL);
}

lv_obj_t* ui_widget_create_back_btn(lv_obj_t* parent, ui_page_id_t target_page){
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 40, 40);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(btn);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    lv_obj_add_event_cb(btn, back_btn_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)target_page);
    return btn;
}

// 页标题控件
lv_obj_t* ui_widget_create_page_title(lv_obj_t* parent, const char* title){
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);
    return label;
}

// 动画结束回调-销毁函数
static void _anim_ready_cb(lv_anim_t * a) {
    lv_obj_t * obj = (lv_obj_t *)a->var;
    lv_obj_del(obj);
}

// 弹窗控件（之前用的 lv_anim_timeline_t 控件实现动画，但是需要手动销毁，而 lv_anim_t 不涉及动态分配）
lv_obj_t* ui_widget_create_toast(const char * msg, uint32_t timeout, lv_event_cb_t custom_cb){
    // 1. 创建基础容器
    lv_obj_t * toast = lv_obj_create(lv_scr_act());
    lv_obj_set_size(toast, 150, 80);
    lv_obj_center(toast);
    lv_obj_set_style_bg_color(toast, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_radius(toast, 10, 0);

    // 绑定删除触发函数
    if(custom_cb){
        lv_obj_add_event_cb(toast, custom_cb, LV_EVENT_DELETE, NULL);
    }
    
    // 添加文字
    lv_obj_t * label = lv_label_create(toast);
    lv_label_set_text(label, msg);
    lv_obj_center(label);

    // 2. 配置“弹出（放大）”动画
    lv_anim_t a_in;
    lv_anim_init(&a_in);
    lv_anim_set_var(&a_in, toast);
    
    lv_anim_set_values(&a_in, 0, 256);  // 这里的缩放是基于样式变换，1.0 = 256
    lv_anim_set_time(&a_in, 300); // 300ms 弹出
    lv_anim_set_exec_cb(&a_in, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
    lv_anim_set_path_cb(&a_in, lv_anim_path_overshoot); // 这种路径会有弹跳感，很灵动
    lv_anim_start(&a_in);

    // 3. 配置“缩小（消失）”动画
    lv_anim_t a_out;
    lv_anim_init(&a_out);
    lv_anim_set_var(&a_out, toast);
    lv_anim_set_values(&a_out, 256, 0); 
    lv_anim_set_time(&a_out, 300);
    // 关键点：设置延迟时间 = (弹出时间 + 用户的停留时间)
    lv_anim_set_delay(&a_out, 300 + timeout); 
    lv_anim_set_exec_cb(&a_out, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
    // 动画结束时调用回调销毁对象
    lv_anim_set_ready_cb(&a_out, _anim_ready_cb);
    lv_anim_start(&a_out);
}