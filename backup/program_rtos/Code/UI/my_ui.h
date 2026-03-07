#ifndef MY_UI_H
#define MY_UI_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "lvgl/lvgl.h"

typedef enum {
    UI_MAIN_SCREEN = 0,
    UI_ENTER_PW_SCREEN,
    UI_CHANGE_PW_SCREEN,
    UI_ADD_FP_SCREEN,
    UI_DELETE_FP_SCREEN,
    UI_ADD_CARD_SCREEN,
    UI_DELETE_CARD_SCREEN
}ui_screen_t;

typedef struct {
    ui_screen_t  page_id;
    int8_t state;
}ui_screen_info_t;

typedef struct ui_msgbox_info{
    ui_screen_t target_scr;
    char* msg_in_box;
    uint8_t ret_to_main;    // 1:跳转主页 0:不跳转主页
    uint8_t close_msgbox;   // 1:关闭窗口 0:保留窗口
    uint8_t has_close_msgbox;   // 1:已关闭 0:未关闭（如果上一次窗口没关闭，则需要在本次先关闭）
}ui_msgbox_info_t;

extern QueueHandle_t queue_msgbox_info;

void my_ui_init(void);

#endif
