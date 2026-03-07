#include <stdlib.h>
#include <string.h>

#include "my_ui.h"
#include "lvgl/lvgl.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "process.h"
#include "data_flash.h"

extern QueueHandle_t xQueue_State;
extern QueueHandle_t xQueue_PCDFlag;
extern QueueHandle_t xQueue_FPFlag;

extern SemaphoreHandle_t xMutex_Flash;

ui_screen_info_t screen_manager;

// SCREEN: ui_main
void ui_main_screen_init(void);
lv_obj_t* ui_main;
lv_obj_t* ui_tabview;
lv_obj_t* ui_tabpage;   //一般来说tabview会有多个tab页
lv_obj_t* ui_to_enterpw;    //跳转到“输入密码”
lv_obj_t* ui_to_changepw;    //跳转到“修改密码”
lv_obj_t* ui_to_addfp;       //跳转到“添加指纹”
lv_obj_t* ui_to_deletefp;    //跳转到“删除指纹”
lv_obj_t* ui_to_addcard;     //跳转到”添加卡号“
lv_obj_t* ui_to_deletecard;  //跳转到“删除卡号”

// SCREEN: ui_enter_pw
void ui_enter_pw_screen_init(void);
lv_obj_t* ui_enter_pw;
lv_obj_t* password_title;
lv_obj_t* ui_backtomain;
uint8_t enterword[PASSWORD_MAX_LEN] = {0};

// SCREEN: ui_change_pw
void ui_change_pw_screen_init(void);
lv_obj_t* ui_change_pw;

// SCREEN: ui_add_fp
void ui_add_fp_screen_init(void);
lv_obj_t* ui_add_fp;
lv_obj_t* ui_msgbox;
#define UI_MSGBOX_H     80
#define UI_MSGBOX_W     200
lv_anim_timeline_t* anim_timeline;

// SCRREN: ui_delete_fp
void ui_delete_fp_screen_init(void);
lv_obj_t* ui_delete_fp;

// SCREEN: ui_add_card
void ui_add_card_screen_init(void);
lv_obj_t* ui_add_card;

// SCREEN: ui_delete_card
void ui_delete_card_screen_init(void);
lv_obj_t* ui_delete_card;

/*** UI处理函数 ***/

static void set_width(void * var, int32_t v){
    lv_obj_set_width((lv_obj_t*)var, v);
}

static void set_height(void * var, int32_t v){
    lv_obj_set_height((lv_obj_t*)var, v);
}

static void set_opa(void * var, int32_t v){
    lv_obj_set_style_opa((lv_obj_t*)var, (lv_opa_t)v, LV_PART_MAIN| LV_STATE_DEFAULT);
}

//删除屏幕（不使用）
void ui_screen_delete( lv_obj_t * target ){
   if(target != NULL)
   {
      lv_obj_del(target);
      target = NULL;    //手动设置为NULL
   }
}

//切换屏幕
void ui_screen_change( lv_obj_t ** target, lv_scr_load_anim_t fademode, int spd, int delay, void (*target_init)(void)) {
//    if(*target == NULL)
      target_init();    //重新初始化
   lv_scr_load_anim(*target, fademode, spd, delay, true);   //最后一个参数用于删除原界面
}

//弹出信息框
void _ui_msgbox_popup(lv_obj_t * parent, char* text){
    char title[] = "HINT:";
    ui_msgbox = lv_msgbox_create(parent, title, text, NULL, false);
    lv_obj_set_x(ui_msgbox, 0);
    lv_obj_set_y(ui_msgbox, 0);
    lv_obj_set_size(ui_msgbox, 2, 1);
    lv_obj_set_align(ui_msgbox, LV_ALIGN_CENTER);

    //创建动画
    lv_anim_t a1;    //宽变化动画
    lv_anim_init(&a1);   //如果传入一个定义出来的指针，会崩溃，不像那些create的
    lv_anim_set_var(&a1, ui_msgbox);
    lv_anim_set_values(&a1, 0, UI_MSGBOX_W);
    lv_anim_set_early_apply(&a1, false);
    lv_anim_set_exec_cb(&a1, set_width);    //设置自定义的执行回调函数
    lv_anim_set_path_cb(&a1, lv_anim_path_bounce ); 
    lv_anim_set_time(&a1, 300);

    lv_anim_t a2;   //高变化动画
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, ui_msgbox);
    lv_anim_set_values(&a2, 0, UI_MSGBOX_H);
    lv_anim_set_early_apply(&a2, false);
    lv_anim_set_exec_cb(&a2, set_height);    //设置自定义的执行回调函数
    lv_anim_set_path_cb(&a2, lv_anim_path_bounce );
    lv_anim_set_time(&a2, 300);

    anim_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_timeline, 0, &a1);
    lv_anim_timeline_add(anim_timeline, 0, &a2);

    lv_anim_timeline_start(anim_timeline);
}

//关闭消息框
void _ui_msgbox_close(void){
    //创建动画
    lv_anim_t a1;    //宽变化动画
    lv_anim_init(&a1);   //如果传入一个定义出来的指针，会崩溃，不像那些create的
    lv_anim_set_var(&a1, ui_msgbox);
    lv_anim_set_values(&a1, UI_MSGBOX_W, 0);
    lv_anim_set_early_apply(&a1, false);
    lv_anim_set_exec_cb(&a1, set_width);    //设置自定义的执行回调函数
    lv_anim_set_path_cb(&a1, lv_anim_path_bounce ); 
    lv_anim_set_time(&a1, 300);

    lv_anim_t a2;   //高变化动画
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, ui_msgbox);
    lv_anim_set_values(&a2, UI_MSGBOX_H, 0);
    lv_anim_set_early_apply(&a2, false);
    lv_anim_set_exec_cb(&a2, set_height);    //设置自定义的执行回调函数
    lv_anim_set_path_cb(&a2, lv_anim_path_bounce );
    lv_anim_set_time(&a2, 300);

    anim_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_timeline, 0, &a1);
    lv_anim_timeline_add(anim_timeline, 0, &a2);

    lv_anim_timeline_start(anim_timeline);
}

/*** 事件回调函数 ***/
void ui_event_backtomain(lv_event_t* e){
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);
    lv_obj_t* ui_current_page = lv_event_get_user_data(e);
    
    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);
}

void ui_event_to_enterpw(lv_event_t* e){
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);

    ui_screen_change(&ui_enter_pw, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_enter_pw_screen_init);
}

void ui_event_to_changepw(lv_event_t* e){
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);

    ui_screen_change(&ui_change_pw, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_change_pw_screen_init);
}

void ui_event_to_addfp(lv_event_t* e){
    
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);

    ui_screen_change(&ui_add_fp, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_add_fp_screen_init);
}

void ui_event_to_deletefp(lv_event_t* e){
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);

    ui_screen_change(&ui_delete_fp, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_delete_fp_screen_init);   
}

void ui_event_to_addcard(lv_event_t* e){
    
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);

    ui_screen_change(&ui_add_card, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_add_card_screen_init);
}

void ui_event_to_deletecard(lv_event_t* e){
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t* event_target = lv_event_get_target(e);

    ui_screen_change(&ui_delete_card, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_delete_card_screen_init);   
}

static void ui_event_keyboard_cb(lv_event_t * e)
{
    lv_obj_t * target = lv_event_get_target(e);
    lv_obj_t * textarea = (lv_obj_t *)lv_event_get_user_data(e);
    const char * text = lv_btnmatrix_get_btn_text(target, lv_btnmatrix_get_selected_btn(target));
	
	static int enter_len = 0;	//当前输入字符数
    static uint8_t status = 0;
	int8_t res = 0;
    
    if(strcmp(text, LV_SYMBOL_BACKSPACE) == 0) {
        lv_textarea_del_char(textarea);
        if(enter_len>0){
            enter_len--;
        }
    }
    else if(strcmp(text, LV_SYMBOL_NEW_LINE) == 0) {
        // 这里可以添加按下Enter键后的处理逻辑
		
		if(screen_manager.page_id == UI_ENTER_PW_SCREEN){
            if(enter_len == password_len && memcmp(enterword, password, password_len) == 0){
                lv_textarea_set_text(textarea, "");
				res = 1;
				xQueueSend(xQueue_State, &res, portMAX_DELAY);
				
            }
            else{
				res = 2;
				xQueueSend(xQueue_State, &res, portMAX_DELAY);
            }	
		}
		
		else if(screen_manager.page_id == UI_CHANGE_PW_SCREEN){
            static uint8_t password_temp[PASSWORD_MAX_LEN] = {0};	//暂存密码
            static uint8_t password_len_temp;
			static int8_t change_state = 0;
            switch(change_state){
                case 0:     //比对原密码
                    if(enter_len == password_len && memcmp(enterword, password, password_len) == 0){
                        lv_label_set_text(password_title, "Please Enter New Password");
                        lv_textarea_set_text(textarea, "");
                        change_state = 1;
                    }
                    else{
                        lv_label_set_text(password_title, "Error! Please Enter Again");
                        lv_textarea_set_text(textarea, "");
                    }
                    break;

                case 1:     //输入新密码
                    password_len_temp = enter_len;
                    for(int i=0; i<enter_len; i++){
                        password_temp[i] = enterword[i];
                    }
                    lv_label_set_text(password_title, "Please Enter New Password Again");
                    lv_textarea_set_text(textarea, "");
                    change_state = 2;
                    break;
                case 2:     //比对新密码
                    if(enter_len == password_len_temp && memcmp(enterword, password_temp, password_len_temp) == 0){
                        //正式修改密码
                        password_len = enter_len;
                        for(int i=0; i<enter_len; i++){
                            password[i] = enterword[i];
                        }
						//保存入flash
						if(xSemaphoreTake(xMutex_Flash, portMAX_DELAY)==pdTRUE){
							data_flash_write();
							xSemaphoreGive(xMutex_Flash);
						}
						
                        lv_textarea_set_text(textarea, "");
                        change_state = 0;
						res = 1;
						xQueueSend(xQueue_State, &res, portMAX_DELAY);

                    }
                    else{
//                        lv_label_set_text(password_title, "Error! Please Enter Again");
                        lv_textarea_set_text(textarea, "");
						res = 2;
						xQueueSend(xQueue_State, &res, portMAX_DELAY);
                    }
                    break;
            }
        }
		enter_len = 0;
    }
    else{
        lv_textarea_add_text(textarea, text);
		enterword[enter_len++] = *text-'0';
    }
}

/*** 任务回调函数  ***/
static void ui_timer_cb(lv_timer_t * user_data){
    static int timer_cnt = 0;   //100ms一次
    xQueueReceive(xQueue_State, &screen_manager.state, 0);

    if(screen_manager.page_id == UI_MAIN_SCREEN){
        return ;
    }
    else if(screen_manager.page_id == UI_ENTER_PW_SCREEN){
        switch(screen_manager.state){
            case 1:     //接收到密码成功时
                _ui_msgbox_popup(ui_enter_pw, "Enter Right");
                screen_manager.state = -1;
                timer_cnt = 0;
				break;
            case 2:     //接收到密码错误时
                _ui_msgbox_popup(ui_enter_pw, "Enter Wrong");
                screen_manager.state = -2;
                timer_cnt = 0;
				break;

            case -1:    //退出回到主页
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);                
                }
				break;
            case -2:    //返回初始化界面
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    _ui_msgbox_close();
                }
                break;  
            default:
                break;
        }
    }
    else if(screen_manager.page_id == UI_CHANGE_PW_SCREEN){
		
        switch(screen_manager.state){
            case 1:     //修改密码成功时
                _ui_msgbox_popup(ui_change_pw, "Change password success");
                screen_manager.state = -1;
                timer_cnt = 0;
				break;
            case 2:     //修改密码错误时
                _ui_msgbox_popup(ui_change_pw, "Change password wrong");
                screen_manager.state = -2;
                timer_cnt = 0;
				break;

            case -1:    //退出回到主页
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);                
                }
				break;
            case -2:    //返回初始化界面
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    _ui_msgbox_close();
                }
                break;  
            default:
                break;
        }
    }

    else if(screen_manager.page_id == UI_ADD_FP_SCREEN){

        switch(screen_manager.state){
            case 0:     //初始化界面
                _ui_msgbox_popup(ui_add_fp, "Please press on your fingerprint");
                screen_manager.state = 1;
                break;
            case 2:     //第一次识别指纹后
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_add_fp, "Please press on your fingerprint AGAIN");
                screen_manager.state = 3;
                break;
            case 4:
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_add_fp, "Add fingerprint Success");
                timer_cnt = 0;
                screen_manager.state = -1;
                break;
            case 5:
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_add_fp, "Add fingerprint Fail");
                timer_cnt = 0;
                screen_manager.state = -2;
                break;

            case -1:    //返回主页
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);
					fp_flag_t temp2 = FP_VERIFY;
					xQueueSend(xQueue_FPFlag, &temp2, 10);					
                }      
            case -2:    //返回初始化界面
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    _ui_msgbox_close();
                }
                break;   
            default:
                break;       
        }
    }

    else if(screen_manager.page_id == UI_DELETE_FP_SCREEN){
        switch(screen_manager.state){
            case 0:     //初始化界面
                _ui_msgbox_popup(ui_delete_fp, "Please press on your fingerprint");
                screen_manager.state = 1;
                break;
            case 2:     //第一次识别指纹后
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_delete_fp, "Please press on your fingerprint AGAIN");
                screen_manager.state = 3;
                break;
            case 4:
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_delete_fp, "Delete fingerprint Success");
                timer_cnt = 0;
                screen_manager.state = -1;
                break;
            case 5:
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_delete_fp, "Delete fingerprint Fail");
                timer_cnt = 0;
                screen_manager.state = -2;
                break;

            case -1:    //返回主页
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);    
					fp_flag_t temp2 = FP_VERIFY;
					xQueueSend(xQueue_FPFlag, &temp2, 10);					
                }      
            case -2:    //返回初始化界面
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    _ui_msgbox_close();
                }
                break;   
            default:
                break;       
        }
    }

    else if(screen_manager.page_id == UI_ADD_CARD_SCREEN){
        switch(screen_manager.state){
            case 0:     //初始化界面
                _ui_msgbox_popup(ui_add_card, "Please put on your card");
                screen_manager.state = 1;
                break;
            case 2:     //接收返回结果：添加成功
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_add_card, "Add card Success");
                timer_cnt = 0;
                screen_manager.state = -1;
                break;
            case 3:     //接收返回结果：添加失败
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_add_fp, "Add card Fail");
                timer_cnt = 0;
                screen_manager.state = -2;
                break;

            case -1:    //返回主页
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);    
					pcd_flag_t temp1 = PCD_CHECK_EXIST;
					xQueueSend(xQueue_PCDFlag, &temp1, 10);				
                }      
            case -2:    //返回初始化界面
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    _ui_msgbox_close();
                }
                break;   
            default:
                break;       
        }
    }

    else if(screen_manager.page_id == UI_DELETE_CARD_SCREEN){
        switch(screen_manager.state){
            case 0:     //初始化界面
                _ui_msgbox_popup(ui_delete_card, "Please put on your card");
                screen_manager.state = 1;
                break;
            case 2:     //接收返回结果：删除成功
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_delete_card, "Delete card Success");
                timer_cnt = 0;
                screen_manager.state = -1;
                break;
            case 3:     //接收返回结果：删除失败
                _ui_msgbox_close();
                _ui_msgbox_popup(ui_delete_card, "Delete card Fail");
                timer_cnt = 0;
                screen_manager.state = -2;
                break;

            case -1:    //返回主页
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
					pcd_flag_t temp1 = PCD_CHECK_EXIST;
					xQueueSend(xQueue_PCDFlag, &temp1, 10);
                    ui_screen_change(&ui_main, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, ui_main_screen_init);                
                }      
            case -2:    //返回初始化界面
                if(timer_cnt >= 15){
                    screen_manager.state = 0;
                    _ui_msgbox_close();
                }
                break;   
            default:
                break;       
        }
    }

    timer_cnt++;    //100ms一次
}

/*** 初始化函数 ***/
void ui_main_screen_init(void){

    ui_main = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_main, lv_color_hex(0x34495e), 0);

    ui_tabview = lv_tabview_create(ui_main, LV_DIR_TOP, 0);     
    lv_obj_set_width(ui_tabview, 220);
    lv_obj_set_height(ui_tabview, 300);
    lv_obj_set_align(ui_tabview, LV_ALIGN_CENTER);
    lv_obj_clear_flag( ui_tabview, LV_OBJ_FLAG_SCROLLABLE ); 

    ui_tabpage = lv_tabview_add_tab(ui_tabview, "tabpage");
    lv_obj_set_style_bg_color(ui_tabpage, lv_color_hex(0x2980b9), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_tabpage, 50, LV_PART_MAIN| LV_STATE_DEFAULT);

    /*Enter Password*/
    ui_to_enterpw = lv_btn_create(ui_tabpage);
    lv_obj_set_size(ui_to_enterpw, 200, 60);
    lv_obj_set_x(ui_to_enterpw, 0);
    lv_obj_set_y(ui_to_enterpw, 0);
    lv_obj_set_align(ui_to_enterpw, LV_ALIGN_TOP_MID);   //对齐似乎是根据父类的大小修改位置，因而上面的xy设置是有效的
    lv_obj_clear_flag( ui_to_enterpw, LV_OBJ_FLAG_SCROLLABLE );

    lv_obj_set_style_bg_color(ui_to_enterpw, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_to_enterpw, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t* ui_label_enterpw = lv_label_create(ui_to_enterpw); //标签已经不属于按键控件本身的内容了
    lv_obj_set_width(ui_label_enterpw, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_enterpw, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_enterpw, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_enterpw, "Enter PassWord");
    lv_obj_set_style_text_color(ui_label_enterpw, lv_color_white(), LV_PART_MAIN| LV_STATE_DEFAULT);

    /*Change Password*/
    ui_to_changepw = lv_btn_create(ui_tabpage);
    lv_obj_set_size(ui_to_changepw, 200, 60);
    lv_obj_set_x(ui_to_changepw, 0);
    lv_obj_set_y(ui_to_changepw, 80);
    lv_obj_set_align(ui_to_changepw, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag( ui_to_changepw, LV_OBJ_FLAG_SCROLLABLE );

    lv_obj_set_style_bg_color(ui_to_changepw, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_to_changepw, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t* ui_label_changepw = lv_label_create(ui_to_changepw);
    lv_obj_set_width(ui_label_changepw, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_changepw, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_changepw, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_changepw, "Change PassWord");
    lv_obj_set_style_text_color(ui_label_changepw, lv_color_white(), LV_PART_MAIN| LV_STATE_DEFAULT);

    /*Add Fingerprint*/
    ui_to_addfp = lv_btn_create(ui_tabpage);
    lv_obj_set_size(ui_to_addfp, 200, 60);
    lv_obj_set_x(ui_to_addfp, 0);
    lv_obj_set_y(ui_to_addfp, 160);
    lv_obj_set_align(ui_to_addfp, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag( ui_to_addfp, LV_OBJ_FLAG_SCROLLABLE );

    lv_obj_set_style_bg_color(ui_to_addfp, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_to_addfp, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t* ui_label_addfp = lv_label_create(ui_to_addfp);
    lv_obj_set_width(ui_label_addfp, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_addfp, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_addfp, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_addfp, "Add Fingerprint");
    lv_obj_set_style_text_color(ui_label_addfp, lv_color_white(), LV_PART_MAIN| LV_STATE_DEFAULT);

    /*Delete Fingerprint*/
    ui_to_deletefp = lv_btn_create(ui_tabpage);
    lv_obj_set_size(ui_to_deletefp, 200, 60);
    lv_obj_set_x(ui_to_deletefp, 0);
    lv_obj_set_y(ui_to_deletefp, 240);
    lv_obj_set_align(ui_to_deletefp, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag( ui_to_deletefp, LV_OBJ_FLAG_SCROLLABLE );
    lv_obj_set_style_bg_color(ui_to_deletefp, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_to_deletefp, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t* ui_label_deletefp = lv_label_create(ui_to_deletefp);
    lv_obj_set_width(ui_label_deletefp, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_deletefp, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_deletefp, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_deletefp, "Delete Fingerprint");
    lv_obj_set_style_text_color(ui_label_deletefp, lv_color_white(), LV_PART_MAIN| LV_STATE_DEFAULT);

    /*Add Cardnum*/
    ui_to_addcard = lv_btn_create(ui_tabpage);
    lv_obj_set_size(ui_to_addcard, 200, 60);
    lv_obj_set_x(ui_to_addcard, 0);
    lv_obj_set_y(ui_to_addcard, 320);
    lv_obj_set_align(ui_to_addcard, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag( ui_to_addcard, LV_OBJ_FLAG_SCROLLABLE );
    lv_obj_set_style_bg_color(ui_to_addcard, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_to_addcard, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t* ui_label_addcard = lv_label_create(ui_to_addcard);
    lv_obj_set_width(ui_label_addcard, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_addcard, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_addcard, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_addcard, "Add Card");
    lv_obj_set_style_text_color(ui_label_addcard, lv_color_white(), LV_PART_MAIN| LV_STATE_DEFAULT);

    /*Delete Cardnum*/
    ui_to_deletecard = lv_btn_create(ui_tabpage);
    lv_obj_set_size(ui_to_deletecard, 200, 60);
    lv_obj_set_x(ui_to_deletecard, 0);
    lv_obj_set_y(ui_to_deletecard, 400);
    lv_obj_set_align(ui_to_deletecard, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag( ui_to_deletecard, LV_OBJ_FLAG_SCROLLABLE );
    lv_obj_set_style_bg_color(ui_to_deletecard, lv_color_hex(0x2c3e50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_to_deletecard, lv_color_hex(0x1abc9c), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t* ui_label_deletecard = lv_label_create(ui_to_deletecard);
    lv_obj_set_width(ui_label_deletecard, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label_deletecard, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label_deletecard, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label_deletecard, "Delete Card");
    lv_obj_set_style_text_color(ui_label_deletecard, lv_color_white(), LV_PART_MAIN| LV_STATE_DEFAULT);

    /*添加事件*/
    lv_obj_add_event_cb(ui_to_enterpw, ui_event_to_enterpw, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_to_changepw, ui_event_to_changepw, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_to_addfp, ui_event_to_addfp, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_to_deletefp, ui_event_to_deletefp, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_to_addcard, ui_event_to_addcard, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_to_deletecard, ui_event_to_deletecard, LV_EVENT_CLICKED, NULL);

    screen_manager.page_id = UI_MAIN_SCREEN;
    screen_manager.state = 0;
}

//输入密码UI页
void ui_enter_pw_screen_init(void){
    ui_enter_pw = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_enter_pw, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_enter_pw, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(ui_enter_pw, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色

    // 页标题
    password_title = lv_label_create(ui_enter_pw);
    lv_label_set_text(password_title, "Please Enter Password");
    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

    // 创建密码文本区域
    lv_obj_t * pwd_ta = lv_textarea_create(ui_enter_pw);
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

    // 创建键盘按钮映射
    static const char * btnm_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
    };

    // 键盘
    lv_obj_t * btnm = lv_btnmatrix_create(ui_enter_pw);
    lv_btnmatrix_set_map(btnm, btnm_map);
    lv_obj_set_size(btnm, 240, 130);
    lv_obj_align(btnm, LV_ALIGN_TOP_MID, 0, 180);

    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(btnm, 0, 0);

    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x2c3e50), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x1abc9c), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btnm, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(btnm, 10, LV_PART_ITEMS);

    //返回键
    ui_backtomain = lv_btn_create(ui_enter_pw);
    lv_obj_set_size(ui_backtomain, 40, 40);
    lv_obj_set_style_bg_color(ui_backtomain, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(ui_backtomain, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(ui_backtomain);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    //添加事件
    lv_obj_add_event_cb(btnm, ui_event_keyboard_cb, LV_EVENT_VALUE_CHANGED, pwd_ta);		// 关键：将键盘与文本区域关联
    lv_obj_add_event_cb(ui_backtomain, ui_event_backtomain, LV_EVENT_CLICKED, ui_enter_pw);

    screen_manager.page_id = UI_ENTER_PW_SCREEN;
    screen_manager.state = 0;
}

//修改密码UI页
void ui_change_pw_screen_init(){
    ui_change_pw = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_change_pw, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_change_pw, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(ui_change_pw, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色

    // 页标题
    password_title = lv_label_create(ui_change_pw);
    lv_label_set_text(password_title, "Please Enter Old Password");
    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

    // 创建密码文本区域
    lv_obj_t * pwd_ta = lv_textarea_create(ui_change_pw);
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

    // 创建键盘按钮映射
    static const char * btnm_map[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
    };

    // 键盘
    lv_obj_t * btnm = lv_btnmatrix_create(ui_change_pw);
    lv_btnmatrix_set_map(btnm, btnm_map);
    lv_obj_set_size(btnm, 240, 130);
    lv_obj_align(btnm, LV_ALIGN_TOP_MID, 0, 180);

    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x34495e), 0);
    lv_obj_set_style_border_width(btnm, 0, 0);

    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x2c3e50), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x1abc9c), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btnm, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(btnm, 10, LV_PART_ITEMS);

    //返回键
    ui_backtomain = lv_btn_create(ui_change_pw);
    lv_obj_set_size(ui_backtomain, 40, 40);
    lv_obj_set_style_bg_color(ui_backtomain, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(ui_backtomain, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(ui_backtomain);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    //添加事件
    lv_obj_add_event_cb(btnm, ui_event_keyboard_cb, LV_EVENT_VALUE_CHANGED, pwd_ta);
    lv_obj_add_event_cb(ui_backtomain, ui_event_backtomain, LV_EVENT_CLICKED, ui_change_pw);

    screen_manager.page_id = UI_CHANGE_PW_SCREEN;
    screen_manager.state = 0;
}

//添加指纹页面
void ui_add_fp_screen_init(){
	fp_flag_t temp = FP_ADD;
	xQueueSend(xQueue_FPFlag, &temp, 10);
	
    ui_add_fp = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_add_fp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_add_fp, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(ui_add_fp, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色

    //返回键
    ui_backtomain = lv_btn_create(ui_add_fp);
    lv_obj_set_size(ui_backtomain, 40, 40);
    lv_obj_set_style_bg_color(ui_backtomain, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(ui_backtomain, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(ui_backtomain);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    // 页标题
    password_title = lv_label_create(ui_add_fp);
    lv_label_set_text(password_title, "Add Fingerprint Page");
    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

    //添加事件
    lv_obj_add_event_cb(ui_backtomain, ui_event_backtomain, LV_EVENT_CLICKED, (lv_obj_t*)ui_add_fp);

    screen_manager.page_id = UI_ADD_FP_SCREEN;
    screen_manager.state = 0;
}

//删除指纹页面
void ui_delete_fp_screen_init(){
    ui_delete_fp = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_delete_fp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_delete_fp, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(ui_delete_fp, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色

    //返回键
    ui_backtomain = lv_btn_create(ui_delete_fp);
    lv_obj_set_size(ui_backtomain, 40, 40);
    lv_obj_set_style_bg_color(ui_backtomain, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(ui_backtomain, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(ui_backtomain);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    // 页标题
    password_title = lv_label_create(ui_delete_fp);
    lv_label_set_text(password_title, "Delete Fingerprint Page");
    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

    //添加事件
    lv_obj_add_event_cb(ui_backtomain, ui_event_backtomain, LV_EVENT_CLICKED, ui_delete_fp);

    screen_manager.page_id = UI_DELETE_FP_SCREEN;
    screen_manager.state = 0;
}

//添加卡号页面
void ui_add_card_screen_init(){
    ui_add_card = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_add_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_add_card, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(ui_add_card, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色

    //返回键
    ui_backtomain = lv_btn_create(ui_add_card);
    lv_obj_set_size(ui_backtomain, 40, 40);
    lv_obj_set_style_bg_color(ui_backtomain, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(ui_backtomain, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(ui_backtomain);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    // 页标题
    password_title = lv_label_create(ui_add_card);
    lv_label_set_text(password_title, "Add Card Page");
    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

    //添加事件
    lv_obj_add_event_cb(ui_backtomain, ui_event_backtomain, LV_EVENT_CLICKED, ui_add_card);

    screen_manager.page_id = UI_ADD_CARD_SCREEN;
    screen_manager.state = 0;
}

//删除卡号页面
void ui_delete_card_screen_init(){
    ui_delete_card = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_delete_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_delete_card, lv_color_hex(0x2c3e50), LV_PART_MAIN|LV_STATE_DEFAULT);		//基础颜色
    lv_obj_set_style_bg_grad_color(ui_delete_card, lv_color_hex(0x3498db), LV_PART_MAIN|LV_STATE_DEFAULT);	//渐变颜色

    //返回键
    ui_backtomain = lv_btn_create(ui_delete_card);
    lv_obj_set_size(ui_backtomain, 40, 40);
    lv_obj_set_style_bg_color(ui_backtomain, lv_color_hex(0x2980b9), 0);
    lv_obj_set_style_radius(ui_backtomain, 10, 0);
    lv_obj_t * backtomain_label = lv_label_create(ui_backtomain);
    lv_label_set_text(backtomain_label, LV_SYMBOL_LEFT);
    lv_obj_center(backtomain_label);

    // 页标题
    password_title = lv_label_create(ui_delete_card);
    lv_label_set_text(password_title, "Delete Card Page");
    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

    //添加事件
    lv_obj_add_event_cb(ui_backtomain, ui_event_backtomain, LV_EVENT_CLICKED, ui_delete_card);

    screen_manager.page_id = UI_DELETE_CARD_SCREEN;
    screen_manager.state = 0;
}

void my_ui_init(void){
    //设置显示器主题
    lv_disp_t *dispp = lv_disp_get_default();   //获取屏幕设备
    // lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_theme_t *theme = lv_theme_mono_init(dispp, false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    //各screen初始化
    ui_main_screen_init();

    //创建软件定时器
    lv_timer_create(ui_timer_cb, 100, NULL);

    //加载screen
    lv_disp_load_scr(ui_main);
}