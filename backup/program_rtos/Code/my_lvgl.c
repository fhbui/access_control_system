//#include <stdlib.h>
//#include <string.h>

//#include "my_lvgl.h"
//#include "lvgl.h"
//#include "process.h"
//#include "dwt.h"

//#include "FreeRTOS.h"
//#include "queue.h"

////函数声明
//static void timer_cb(lv_timer_t * timer); // lv_timer_t 是 struct _lv_timer_t 的typedef
//static void navigate_back(lv_event_t * e);
//static void keyboard_cb(lv_event_t * e);
//static void btn_to_password_cb(lv_event_t * e);
//static void btn_to_changeword_cb(lv_event_t * e);
//static void btn_to_addfinger_cb(lv_event_t * e);
//static void btn_to_addcard_cb(lv_event_t * e);
//static void btn_to_delfinger_cb(lv_event_t * e);
//static void btn_to_delcard_cb(lv_event_t * e);

////全局变量
//static page_manager_t page_manager;
//static page_info_t page_info;
//static lv_obj_t * password_title;
//static lv_obj_t * verify_text_area;

//extern QueueHandle_t xQueue_PCDFlag;
//extern QueueHandle_t xQueue_FPFlag;

///**
//  * @brief  页面切换函数
//  * @param	页对象
//  * @retval 无
//  */
//void switch_page(page_t page) {
//    if(page >= PAGE_COUNT) return;
//    
//    // 加载目标页面
//    lv_scr_load(page_manager.screens[page]);
//    page_manager.current_page = page;
//}

///**
//  * @brief  创建主页面
//  * @param	父类对象
//  * @retval 无
//  */
//void create_main_page(lv_obj_t* parent){
//    // 创建主容器
//    lv_obj_t * cont = lv_obj_create(parent);
//    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));	//LV_PCT(100)就是100%
//    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2c3e50), 0);		//基础颜色
//    lv_obj_set_style_bg_grad_color(cont, lv_color_hex(0x3498db), 0);	//渐变颜色
//    lv_obj_set_style_bg_grad_dir(cont, LV_GRAD_DIR_VER, 0);		//渐变方向
//    lv_obj_set_style_border_width(cont, 0, 0);	//设置对象的边框宽度为 0
//    lv_obj_set_style_radius(cont, 0, 0);		//设置对象的圆角半径为 0
//    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);	//移除对象的可滚动标志：LVGL 对象通常可以滚动（如果内容超出显示区域）
//    lv_obj_center(cont);	//以父级中心居中

//    //按钮对象
//    lv_obj_t * btn_to_password = lv_btn_create(cont);
//    lv_obj_set_size(btn_to_password, 200, 40);
//    lv_obj_align(btn_to_password, LV_ALIGN_TOP_MID, 0, 10);
//    lv_obj_t * label = lv_label_create(btn_to_password);
//    lv_label_set_text(label, "Pass Word");
//    lv_obj_center(label);
//    lv_obj_add_event_cb(btn_to_password, btn_to_password_cb, LV_EVENT_CLICKED, NULL);    //最后一个参数用于lv_event_get_user_data

//    lv_obj_t * btn_to_changeword = lv_btn_create(cont);
//    lv_obj_set_size(btn_to_changeword, 200, 40);
//    lv_obj_align(btn_to_changeword, LV_ALIGN_TOP_MID, 0, 60); 
//    label = lv_label_create(btn_to_changeword);
//    lv_label_set_text(label, "Change Word");
//    lv_obj_center(label);
//    lv_obj_add_event_cb(btn_to_changeword, btn_to_changeword_cb, LV_EVENT_CLICKED, NULL);

//    lv_obj_t * btn_to_addfinger = lv_btn_create(cont);
//    lv_obj_set_size(btn_to_addfinger, 200, 40);
//    lv_obj_align(btn_to_addfinger, LV_ALIGN_TOP_MID, 0, 110);
//    label = lv_label_create(btn_to_addfinger);
//    lv_label_set_text(label, "Add Finger");
//    lv_obj_center(label);
//    lv_obj_add_event_cb(btn_to_addfinger, btn_to_addfinger_cb, LV_EVENT_CLICKED, NULL);

//    lv_obj_t * btn_to_addcard = lv_btn_create(cont);
//    lv_obj_set_size(btn_to_addcard, 200, 40);
//    lv_obj_align(btn_to_addcard, LV_ALIGN_TOP_MID, 0, 160); 
//    label = lv_label_create(btn_to_addcard);
//    lv_label_set_text(label, "Add Card");
//    lv_obj_center(label);
//    lv_obj_add_event_cb(btn_to_addcard, btn_to_addcard_cb, LV_EVENT_CLICKED, NULL);

//    lv_obj_t * btn_to_delfinger = lv_btn_create(cont);
//    lv_obj_set_size(btn_to_delfinger, 200, 40);
//    lv_obj_align(btn_to_delfinger, LV_ALIGN_TOP_MID, 0, 210);
//    label = lv_label_create(btn_to_delfinger);
//    lv_label_set_text(label, "Delete Finger");
//    lv_obj_center(label);
//    lv_obj_add_event_cb(btn_to_delfinger, btn_to_delfinger_cb, LV_EVENT_CLICKED, NULL);

//    lv_obj_t * btn_to_delcard = lv_btn_create(cont);
//    lv_obj_set_size(btn_to_delcard, 200, 40);
//    lv_obj_align(btn_to_delcard, LV_ALIGN_TOP_MID, 0, 260);    
//    label = lv_label_create(btn_to_delcard);
//    lv_label_set_text(label, "Delete Card");
//    lv_obj_center(label);
//    lv_obj_add_event_cb(btn_to_delcard, btn_to_delcard_cb, LV_EVENT_CLICKED, NULL);   
//}

///**
//  * @brief  创建输入密码界面
//  * @param	父类对象
//  * @retval 无
//  */
//void create_password_page(lv_obj_t * parent)
//{
//    // 创建主容器
//    lv_obj_t * cont = lv_obj_create(parent);
//    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));	//LV_PCT(100)就是100%
//    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2c3e50), 0);		//基础颜色
//    lv_obj_set_style_bg_grad_color(cont, lv_color_hex(0x3498db), 0);	//渐变颜色
//    lv_obj_set_style_bg_grad_dir(cont, LV_GRAD_DIR_VER, 0);		//渐变方向
//    lv_obj_set_style_border_width(cont, 0, 0);	//设置对象的边框宽度为 0
//    lv_obj_set_style_radius(cont, 0, 0);		//设置对象的圆角半径为 0
//    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);	//移除对象的可滚动标志：LVGL 对象通常可以滚动（如果内容超出显示区域）
//    lv_obj_center(cont);	//以父级中心居中

//    // 创建标题
//    password_title = lv_label_create(cont);
//    lv_label_set_text(password_title, "Please Enter PassWord");
//    lv_obj_set_style_text_color(password_title, lv_color_white(), 0);
//    lv_obj_set_style_text_font(password_title, &lv_font_montserrat_14, 0);
//    lv_obj_align(password_title, LV_ALIGN_TOP_MID, 0, 40);

//    // 创建密码文本区域
//    lv_obj_t * pwd_ta = lv_textarea_create(cont);
//    lv_obj_set_size(pwd_ta, 200, 60);
//    lv_obj_align(pwd_ta, LV_ALIGN_TOP_MID, 0, 100);
//    lv_textarea_set_text(pwd_ta, "");
//    lv_textarea_set_password_mode(pwd_ta, true);
//    lv_textarea_set_one_line(pwd_ta, true);	//设置文本区域为单行模式
//    lv_textarea_set_max_length(pwd_ta, 8);
//    lv_obj_set_style_bg_color(pwd_ta, lv_color_hex(0x34495e), 0);
//    lv_obj_set_style_text_color(pwd_ta, lv_color_white(), 0);
//    lv_obj_set_style_border_width(pwd_ta, 2, 0);	//边框厚度
//    lv_obj_set_style_border_color(pwd_ta, lv_color_hex(0x1abc9c), 0);	//边框颜色

//    // 创建键盘按钮映射
//    static const char * btnm_map[] = {
//        "1", "2", "3", "\n",
//        "4", "5", "6", "\n",
//        "7", "8", "9", "\n",
//        LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
//    };

//    // 创建键盘
//    lv_obj_t * btnm = lv_btnmatrix_create(cont);
//    lv_btnmatrix_set_map(btnm, btnm_map);
//    lv_obj_set_size(btnm, 240, 130);
//    lv_obj_align(btnm, LV_ALIGN_TOP_MID, 0, 180);
//    lv_obj_add_event_cb(btnm, keyboard_cb, LV_EVENT_VALUE_CHANGED, pwd_ta);		// 关键：将键盘与文本区域关联
//    
//    // 设置键盘样式
//    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x34495e), 0);
//    lv_obj_set_style_border_width(btnm, 0, 0);
//    
//    // 设置按钮样式
//    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x2c3e50), LV_PART_ITEMS);
//    lv_obj_set_style_bg_color(btnm, lv_color_hex(0x1abc9c), LV_PART_ITEMS | LV_STATE_PRESSED);
//    lv_obj_set_style_text_color(btnm, lv_color_white(), LV_PART_ITEMS);
//    lv_obj_set_style_radius(btnm, 10, LV_PART_ITEMS);

//    // 设置返回按钮
//    lv_obj_t * back_btn = lv_btn_create(cont);
//    lv_obj_set_size(back_btn, 30, 30);
//    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x2980b9), 0);
//    lv_obj_set_style_radius(back_btn, 20, 0);
//    
//    lv_obj_t * back_label = lv_label_create(back_btn);
//    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
//    lv_obj_center(back_label);
//    
//    lv_obj_add_event_cb(back_btn, navigate_back, LV_EVENT_CLICKED, pwd_ta);
//}

///**
//  * @brief  创建检验界面
//  * @param	父类对象
//  * @retval 无
//  */
//void create_verify_page(lv_obj_t * parent){
//    // 创建主容器
//    lv_obj_t * cont = lv_obj_create(parent);
//    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));	//LV_PCT(100)就是100%
//    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2c3e50), 0);		//基础颜色
//    lv_obj_set_style_bg_grad_color(cont, lv_color_hex(0x3498db), 0);	//渐变颜色
//    lv_obj_set_style_bg_grad_dir(cont, LV_GRAD_DIR_VER, 0);		//渐变方向
//    lv_obj_set_style_border_width(cont, 0, 0);	//设置对象的边框宽度为 0
//    lv_obj_set_style_radius(cont, 0, 0);		//设置对象的圆角半径为 0
//    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);	//移除对象的可滚动标志：LVGL 对象通常可以滚动（如果内容超出显示区域）
//    lv_obj_center(cont);	//以父级中心居中

//    // 设置返回按钮
//    lv_obj_t * back_btn = lv_btn_create(cont);
//    lv_obj_set_size(back_btn, 30, 30);
//    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x2980b9), 0);
//    lv_obj_set_style_radius(back_btn, 20, 0);
//    
//    lv_obj_t * back_label = lv_label_create(back_btn);
//    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
//    lv_obj_center(back_label);
//    
//    lv_obj_add_event_cb(back_btn, navigate_back, LV_EVENT_CLICKED, NULL);

//    // 创建文本区域
//    verify_text_area = lv_textarea_create(cont);
//    lv_obj_set_size(verify_text_area, 200, 60);
//    lv_obj_align(verify_text_area, LV_ALIGN_TOP_MID, 0, 100);
//    lv_textarea_set_text(verify_text_area, "");
//    lv_obj_set_style_bg_color(verify_text_area, lv_color_hex(0x34495e), 0);
//    lv_obj_set_style_text_color(verify_text_area, lv_color_white(), 0);
//    lv_obj_set_style_border_width(verify_text_area, 2, 0);	//边框厚度
//    lv_obj_set_style_border_color(verify_text_area, lv_color_hex(0x1abc9c), 0);	//边框颜色   
//}

////定时器回调函数（50ms）
//static void timer_cb(lv_timer_t * timer) {
//    // if (current_state != previous_state) {
//    //     update_ui_for_state(current_state);
//    //     previous_state = current_state;
//    // }
//}

////返回键回调函数
//static void navigate_back(lv_event_t * e){
//    //对于复杂工程，可以使用页面历史堆栈来进行回退
//        if(page_info.task == ENTER_PASSWORD || page_info.task == CHANGE_PASSWORD){
//            //清除输入框
//            lv_obj_t * textarea = (lv_obj_t *)lv_event_get_user_data(e);
//            lv_textarea_set_text(textarea, "");
//        }
//        switch_page(MAIN_PAGE);
//        page_info.task = IDLE;
//        page_info.status = 0;
//		
//		pcd_flag = PCD_CHECK_EXIST;
//}

////点击“输入密码”回调函数
//static void btn_to_password_cb(lv_event_t * e){
//    lv_event_code_t code = lv_event_get_code(e);    //事件被触发时会创建这个事件对象
//    if(code == LV_EVENT_CLICKED) {
//        switch_page(PASSWORD_PAGE);
//        lv_label_set_text(password_title, "Please Enter PassWord");
//        page_info.task = ENTER_PASSWORD;
//        page_info.status = 0;
//    }
//}

////点击“修改密码”回调函数
//static void btn_to_changeword_cb(lv_event_t * e){
//    switch_page(PASSWORD_PAGE);
//    // 重新创建标题
//    lv_label_set_text(password_title, "Please Enter Old PassWord");
//    page_info.task = CHANGE_PASSWORD;
//    page_info.status = 0;
//}

////点击“添加指纹”回调函数
//static void btn_to_addfinger_cb(lv_event_t * e){
//    switch_page(VERIFY_PAGE);
//    lv_textarea_set_text(verify_text_area, "please press your finger");
//    page_info.task = ADD_FINGER;
//    page_info.status = 0;
//	
//	fp_flag_t temp = FP_ADD;
//	xQueueSend(xQueue_FPFlag, &temp, 10);
//}

////点击“添加卡号”回调函数
//static void btn_to_addcard_cb(lv_event_t * e){
//    switch_page(VERIFY_PAGE);
//    lv_textarea_set_text(verify_text_area, "please put on your card");
//    page_info.task = ADD_CARD;
//    page_info.status = 0;
//	
//	pcd_flag_t temp = PCD_ADD_CARD;
//	xQueueSend(xQueue_PCDFlag, &temp, 10);
//}

////点击“删除指纹”回调函数
//static void btn_to_delfinger_cb(lv_event_t * e){
//    switch_page(VERIFY_PAGE);
//    lv_textarea_set_text(verify_text_area, "please press your finger");
//    page_info.task = DELETE_FINGER;
//    page_info.status = 0;    
//	
//	fp_flag_t temp = FP_DELETE;
//	xQueueSend(xQueue_FPFlag, &temp, 10);
//}

////点击“删除卡号”回调函数
//static void btn_to_delcard_cb(lv_event_t * e){
//    switch_page(VERIFY_PAGE);
//    lv_textarea_set_text(verify_text_area, "please put on your card");
//    page_info.task = DELETE_CARD;
//    page_info.status = 0;
//	
//	pcd_flag_t temp = PCD_DELETE_CARD;
//	xQueueSend(xQueue_PCDFlag, &temp, 10);
//}

//// 键盘按钮回调函数
//uint8_t enterword[PASSWORD_MAX_LEN] = {0};
////uint8_t password[MAX_LEN] = {1,2,3,4,5,6,7,8};  //启动后要从flash中更新
////uint8_t password_len = 8;   //启动后要从flash中更新

//static void keyboard_cb(lv_event_t * e)
//{
//    lv_obj_t * target = lv_event_get_target(e);
//    lv_obj_t * textarea = (lv_obj_t *)lv_event_get_user_data(e);
//    const char * text = lv_btnmatrix_get_btn_text(target, lv_btnmatrix_get_selected_btn(target));
//	
//	static int index = 0;
//    static uint8_t status = 0;
//    
//    if(strcmp(text, LV_SYMBOL_BACKSPACE) == 0) {
//        lv_textarea_del_char(textarea);
//        if(index>0){
//            index--;
//        }
//    }
//    else if(strcmp(text, LV_SYMBOL_NEW_LINE) == 0) {
//        // 这里可以添加按下Enter键后的处理逻辑
//        if(page_info.task == ENTER_PASSWORD){
//            if(index == password_len && memcmp(enterword, password, password_len) == 0){
//                lv_textarea_set_text(textarea, "");
//                switch_page(VERIFY_PAGE);
//                lv_textarea_set_text(verify_text_area, "Opening the door...");
//                //延时
////                for(int i=5000; i>0; i--);

//                switch_page(MAIN_PAGE);
//            }
//            else{
//                lv_label_set_text(password_title, "Error! Please Enter Again");
//                lv_textarea_set_text(textarea, "");
//            }
//        }
//        else if(page_info.task == CHANGE_PASSWORD){
//            static uint8_t password_temp[PASSWORD_MAX_LEN] = {0};
//            static uint8_t password_len_temp;
//            switch(page_info.status){
//                case 0:     //比对原密码
//                    if(index == password_len && memcmp(enterword, password, password_len) == 0){
//                        lv_label_set_text(password_title, "Please Enter New Password");
//                        lv_textarea_set_text(textarea, "");
//                        page_info.status = 1;
//                    }
//                    else{
//                        lv_label_set_text(password_title, "Error! Please Enter Again");
//                        lv_textarea_set_text(textarea, "");
//                    }
//                    break;

//                case 1:     //输入新密码
//                    password_len_temp = index;
//                    for(int i=0; i<index; i++){
//                        password_temp[i] = enterword[i];
//                    }
//                    lv_label_set_text(password_title, "Please Enter New Password Again");
//                    lv_textarea_set_text(textarea, "");
//                    page_info.status = 2;
//                    break;
//                case 2:     //比对新密码
//                    if(index == password_len_temp && memcmp(enterword, password_temp, password_len_temp) == 0){
//                        //正式修改密码
//                        password_len = index;
//                        for(int i=0; i<index; i++){
//                            password[i] = enterword[i];
//                        }

//                        lv_textarea_set_text(textarea, "");
//                        page_info.status = 0;
//                        switch_page(VERIFY_PAGE);
//                        lv_textarea_set_text(verify_text_area, "Set New Password Success");
//                        //延时
////                        for(int i=5000; i>0; i--);
//						
//                        switch_page(MAIN_PAGE);
//                    }
//                    else{
//                        lv_label_set_text(password_title, "Error! Please Enter Again");
//                        lv_textarea_set_text(textarea, "");
//                    }
//                    break;
//            }
//        }
//        index = 0;  //按下Enter后清除长度记录
//    }
//    else{
//        lv_textarea_add_text(textarea, text);
//			if(index < PASSWORD_MAX_LEN){
//			enterword[index++] = *text-'0';
//		}
//    }
//}

///**
//  * @brief  初始化页面管理器
//  * @param	页对象
//  * @retval 无
//  */
//void page_manager_init(lv_obj_t* parent) {
//    // 创建所有页面
//    for(int i = 0; i < PAGE_COUNT; i++) {
//        page_manager.screens[i] = lv_obj_create(parent);
//    }
//    
//    // 创建各个页面的内容
//    create_main_page(page_manager.screens[MAIN_PAGE]);  //还好各个界面的父类都被存储起来
//    create_password_page(page_manager.screens[PASSWORD_PAGE]);
//    create_verify_page(page_manager.screens[VERIFY_PAGE]);

//    //创建定时器
//    lv_timer_t * state_check_timer = lv_timer_create(timer_cb , 50, NULL); // 每50ms检查一次状态变化
//    
//    // 加载主页面
//    switch_page(MAIN_PAGE);
//}
