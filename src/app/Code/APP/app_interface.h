#ifndef __APP_INTERFACE_H
#define __APP_INTERFACE_H

#include "freertos.h"
#include "queue.h"

typedef enum{
    INTERFACE_UI_TOAST_SHOW = 0,
    INTERFACE_UI_TOAST_SHOW_AND_RETURN,
}interface_ui_type_t;

typedef struct {
    interface_ui_type_t type;
    char* usr_data;
}interface_ui_msg_t;

extern QueueHandle_t ui_msg_queue;     // 定义在app_main.c

#endif