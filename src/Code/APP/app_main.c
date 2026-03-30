#include <stdio.h>
#include "bsp_uart.h"
#include "ft6336.h"
#include "ili9341.h"
#include "rc522.h"
#include "bsp_dwt.h"
#include "ui_main.h"
#include "lvgl.h"
#include "log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static const char* TAG = "app_main";

TaskHandle_t task_card_handle;
TaskHandle_t task_fp_handle;

static void task_ui(void *pvParameters);
static void task_card(void *pvParameters);
static void task_fp(void *pvParameters);
static void task_door(void *pvParameters);
static void task_feed_wdg(void *pvParameters);

QueueHandle_t ui_msg_queue;

void app_main(void){
	bsp_InitDWT();
//	ili9341_init();
//	ili9341_set_direction(2);
//	ili9341_fill(0, 0, 50, 50, ILI9341_BLUE);
//	rc522_init();
	// ft6336_init();
	// uint8_t uid[4] = {0};
	// ft6336_data_t data;
	
	xTaskCreate(task_ui, "ui_task", 512, NULL, 3, NULL);
	xTaskCreate(task_card, "card_task", 128, NULL, 3, NULL);
	xTaskCreate(task_fp, "fp_task", 128, NULL, 3, NULL);
	xTaskCreate(task_door, "door_task", 128, NULL, 2, NULL);
	xTaskCreate(task_feed_wdg, "wdg_task", 128, NULL, 1, NULL);

#if 0
	ui_init();
	while(1){
		lv_timer_handler();
		bsp_Delayms(5);
		// bsp_Delayms(50);
		// ft6336_read_points(&data);
		// if(data.point_num > 0 && data.point_num <= FT6336_MAX_TOUCH){
		// 	LOG_DEBUG(TAG, "x is %d, y is %d", data.points[0].x, data.points[0].y);
		// }
//		if(rc522_identify_card(uid) == RC522_OK){
//			LOG_DEBUG(TAG, "%d %d %d %d", uid[0], uid[1], uid[2], uid[3]);
//		}
	}
#endif
}

static void task_ui(void *pvParameters){
	ui_init();
	while(1){
		lv_timer_handler();
		vTaskDelay(5);
	}
}

static void task_card(void *pvParameters){
	rc522_init();
	uint32_t notified_val = 0;
	while(1){
		xTaskNotifyWait(0x00, 0xFFFFFFFF, &notified_val, 10);
		switch(notified_val){
			case 0:
				LOG_DEBUG(TAG, "card verify state");
				break;
			case 1:
				LOG_DEBUG(TAG, "card add state");
				break;
			case 2:
				LOG_DEBUG(TAG, "card delete state");
				break;
			default:
				LOG_DEBUG(TAG, "card error state");
				break;
		}
	}
}

static void task_fp(void *pvParameters){
	while(1){
		
	}
}

static void task_door(void *pvParameters){
	while(1){
		
	}
}

static void task_feed_wdg(void *pvParameters){
	while(1){
		
	}
}