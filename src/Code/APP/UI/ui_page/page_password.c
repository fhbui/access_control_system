#include "lvgl.h"
#include "ui_widgets.h"
#include "ui_service.h"

#define PWD_MAX_LEN     20

// 工作状态
typedef enum {
    PAGE_STATE_VERIFY_PWD = 0,
    PAGE_STATE_ENTER_OLD_PWD,
    PAGE_STATE_ENTER_NEW_PWD,
    PAGE_STATE_ENTER_NEW_PWD_AGAIN,
}_page_pwd_state_t;

// 该页面所维护的结构体句柄
typedef struct {
    int pwd_len;
    int pwd_buf[PWD_MAX_LEN];    // 已有密码
    // int enter_len;
    // int enter_buf[PWD_MAX_LEN];
    _page_pwd_state_t status;

    lv_obj_t* title;    
}_page_pwd_handle_t;

// 全局变量
static _page_pwd_handle_t handle;


// 键盘回调函数
static void ui_event_keyboard_cb(lv_event_t * e){
    lv_obj_t * target = lv_event_get_target(e);
    lv_obj_t * textarea = (lv_obj_t *)lv_event_get_user_data(e);
    const char * text = lv_btnmatrix_get_btn_text(target, lv_btnmatrix_get_selected_btn(target));
	
    static int enter_len = 0;	//当前输入字符数
    static int enter_buf[PWD_MAX_LEN];
    static int temp_buf[PWD_MAX_LEN];
    static int temp_len;
    
    // 回退键处理
    if(strcmp(text, LV_SYMBOL_BACKSPACE) == 0) {
        if(enter_len>0){
            lv_textarea_del_char(textarea);
            enter_len--;
        }
        return ;
    }

    // ENTER键处理
    else if(strcmp(text, LV_SYMBOL_NEW_LINE) == 0) {
        bool is_correct = false;

        switch(handle.status){
            case PAGE_STATE_VERIFY_PWD:
                is_correct = (enter_len == handle.pwd_len && memcmp(enter_buf, handle.pwd_buf, handle.pwd_len) == 0);
                if(is_correct){
                    // 发布开锁信号
                    ui_service_require(UI_SERVICE_OPEN_DOOR, NULL);
                    enter_len = 0;
                    // 调用弹窗，结束后回到主页
                    ui_widget_create_toast("password correct", 1000, lv_router_return_event_cb);
                    return ;
                }
                break;

            case PAGE_STATE_ENTER_OLD_PWD:
                is_correct = (enter_len == handle.pwd_len && memcmp(enter_buf, handle.pwd_buf, handle.pwd_len) == 0);
                if(is_correct){
                    handle.status = PAGE_STATE_ENTER_NEW_PWD;   // 更新工作状态
                    lv_label_set_text(handle.title, "Please Enter New Password");
                    enter_len = 0;
                    lv_textarea_set_text(textarea, "");
                    return ;
                }
                break;

            case PAGE_STATE_ENTER_NEW_PWD:
                memcpy(temp_buf, enter_buf, enter_len);
                temp_len = enter_len;
                handle.status = PAGE_STATE_ENTER_NEW_PWD_AGAIN;
                lv_label_set_text(handle.title, "Please Enter New Password Again");
                return ;

            case PAGE_STATE_ENTER_NEW_PWD_AGAIN:
                is_correct = (enter_len == temp_len && memcmp(enter_buf, temp_buf, temp_len) == 0);
                if(is_correct){
                    enter_len = 0;
                    // 调用弹窗，结束后回到主页
                    ui_widget_create_toast("change password success", 1000, lv_router_return_event_cb);
                    return ;                
                }
                break;
        }
        // 运行到这里，说明前面失败
        ui_widget_create_toast("error", 1000, NULL);
        return ;
    }
    
    // 其他处理
    else if(enter_len < PWD_MAX_LEN){
        lv_textarea_add_text(textarea, text);
        enter_buf[enter_len++] = *text-'0';
    }
}


// 设计函数
static lv_obj_t* _create_pwd_area(lv_obj_t* parent){
    lv_obj_t * pwd_ta = lv_textarea_create(parent);
    lv_obj_set_size(pwd_ta, 200, 60);
    lv_obj_align(pwd_ta, LV_ALIGN_TOP_MID, 0, 100);
    lv_textarea_set_text(pwd_ta, "");
    lv_textarea_set_password_mode(pwd_ta, true);
    lv_textarea_set_one_line(pwd_ta, true);	//设置文本区域为单行模式
    lv_textarea_set_max_length(pwd_ta, 8);
    lv_obj_set_style_bg_color(pwd_ta, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_text_color(pwd_ta, lv_color_white(), 0);
    lv_obj_set_style_border_width(pwd_ta, 2, 0);	//边框厚度
    lv_obj_set_style_border_color(pwd_ta, lv_color_hex(0x1abc9c), 0);	//边框颜色

    return pwd_ta;
}

static lv_obj_t* _create_keyboard(lv_obj_t* parent){
    static const char * btnm_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
    };

    // 键盘
    lv_obj_t * btnm = lv_btnmatrix_create(parent);
    lv_btnmatrix_set_map(btnm, btnm_map);
    lv_obj_set_size(btnm, 240, 130);
    lv_obj_align(btnm, LV_ALIGN_TOP_MID, 0, 180);

    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(btnm, 0, 0);

    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x2c3e50), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x1abc9c), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btnm, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(btnm, 10, LV_PART_ITEMS);

    return btnm;
}

// 页面初始化
void page_password_init(int status){
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    ui_widget_apply_screen_style(scr);

    ui_widget_create_back_btn(scr, UI_PAGE_MAIN);
    lv_obj_t* pwd_ta = _create_pwd_area(scr);
    lv_obj_t* btnm = _create_keyboard(scr);
    lv_obj_add_event_cb(btnm, ui_event_keyboard_cb, LV_EVENT_VALUE_CHANGED, pwd_ta);

    if(status == 0){    // 输入密码工作流
        handle.title = ui_widget_create_page_title(scr, "Please Enter Password");
        handle.status = PAGE_STATE_VERIFY_PWD;
    }
    else if(status == 1){   // 修改密码工作流
        handle.title = ui_widget_create_page_title(scr, "Please Enter Old Password");
        handle.status = PAGE_STATE_ENTER_OLD_PWD;
    }

    // 获取已有密码信息
    ui_service_require(UI_SERVICE_LOAD_PWD, (uint32_t*)handle.pwd_buf);
    // 页面加载
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);     // 不销毁主页状态
}