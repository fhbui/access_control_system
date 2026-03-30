// 对外服务函数。UI层和外部交互的函数全放在这里
#include "lvgl.h"
#include "ui_service.h"
#include "ui_widgets.h"
#include "ui_router.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 相应外层包含
#include "app_interface.h"

extern TaskHandle_t task_card_handle;
extern TaskHandle_t task_fp_handle;

// 模块消息映射表


static QueueHandle_t _service_type_queue;

bool ui_service_require(ui_service_type_t type, uint32_t* param){
    switch(type){
        case UI_SERVICE_LOAD_PWD:
            // 从Flash读取密码并写入param
            break;
        case UI_SERVICE_SAVE_PWD:
            // 将param保存入Flash
            break;
        case UI_SERVICE_SET_FINGER_VERIFY:
            // 设置指纹模块任务的任务通知
            xTaskNotify(task_fp_handle, 0, eSetValueWithOverwrite);
            break;
        case UI_SERVICE_SET_FINGER_ADD:
            // 设置指纹模块任务的任务通知
            xTaskNotify(task_fp_handle, 1, eSetValueWithOverwrite);
            break;
        case UI_SERVICE_SET_FINGER_DEL:
            // 设置指纹模块任务的任务通知
            xTaskNotify(task_fp_handle, 2, eSetValueWithOverwrite);
            break;
        case UI_SERVICE_SET_CARD_VERIFY:
            // 设置读卡模块任务的任务通知
            xTaskNotify(task_card_handle, 0, eSetValueWithOverwrite);
            break;
        case UI_SERVICE_SET_CARD_ADD:
            // 设置读卡模块任务的任务通知
            xTaskNotify(task_card_handle, 1, eSetValueWithOverwrite);
            break;
        case UI_SERVICE_SET_CARD_DEL:
            // 设置读卡模块任务的任务通知
            xTaskNotify(task_card_handle, 2, eSetValueWithOverwrite);
            break;
		default:
			break;
    }
    return true;
}

// 任务中不调用lv_obj相关函数
// static void task_ui_service_out_proc(void *pvParameters){
//     ui_service_type_t type;

//     while(1){
//         if(xQueueReceive(_service_type_queue, &type, portMAX_DELAY) != pdPASS){
//             continue;
//         }

//         switch(type){
//             case UI_SERVICE_LOAD_PWD:
//             // 具体操作封装成函数
//             case UI_SERVICE_SAVE_PWD:
//             case UI_SERVICE_SET_FINGER_VERIFY:
//             case UI_SERVICE_SET_FINGER_ADD:
//             case UI_SERVICE_SET_FINGER_DEL:
//             case UI_SERVICE_SET_CARD_VERIFY:
//             case UI_SERVICE_SET_CARD_ADD:
//             case UI_SERVICE_SET_CARD_DEL:
//         }
//     }
// }

// static void task_ui_service_in_proc(void *pvParameters){

// }

static void _ui_service_timer_cb(lv_timer_t * user_data){
    // 接收队列消息
    interface_ui_msg_t msg;
    if(xQueueReceive(ui_msg_queue, &msg, 10) != pdPASS){
        return ;
    }

    if(msg.type == INTERFACE_UI_TOAST_SHOW){
        ui_widget_create_toast((char*)msg.usr_data, 1000, NULL);
    }
    else if(msg.type == INTERFACE_UI_TOAST_SHOW_AND_RETURN){
        ui_widget_create_toast((char*)msg.usr_data, 1000, lv_router_return_event_cb);
    }
}

void ui_service_init(void){
    // _service_type_queue = xQueueCreate(1, sizeof(ui_service_type_t));
    // 创建后台输入输出任务
    // xTaskCreate(task_ui_service_out_proc, "ui_service_out_proc", 128, NULL, 2, NULL);
    // xTaskCreate(task_ui_service_in_proc, "ui_service_in_proc", 128, NULL, 2, NULL);

    // 创建定时器来检测模块输入
    lv_timer_create(_ui_service_timer_cb, 100, NULL);
}
