#include "main.h"
#include "lvgl.h"
#include "my_lvgl.h"
#include "process.h"

#include "FreeRTOS.h"
#include "task.h"

/******************** 全局变量 *********************/
volatile uint32_t ulHighFrequencyTimerTicks;	//用于检测栈使用情况
static TaskHandle_t xHandleSysInfo;
static TaskHandle_t xHandleLVGL;

/******************** 函数声明 *********************/
static void vTaskSysInfo(void *pvParameters);
static void vTaskLVGL(void *pvParameters);


/**
  * @brief  检测任务栈溢出函数
  * @param	任务句柄、函数名
  * @retval 无
  */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
	printf("任务：%s 发现栈溢出\r\n", pcTaskName);
}

/**
  * @brief  SysTick钩子函数
  * @retval 无
  */
void vApplicationTickHook(void){
	lv_tick_inc(1);
}

/**
  * @brief  freertos初始化/启用函数
  * @retval 无
  */
void freertos_init(void){
	
	/**/
	
	/*创建任务*/
	xTaskCreate(vTaskSysInfo, "vTaskSysInfo", 256, NULL, 2, &xHandleSysInfo);
//	xTaskCreate(vTaskLVGL, "vTaskLVGL", 512, NULL, 4, &xHandleLVGL);
	
	vTaskStartScheduler();	//启动OS调度机制
}


static void vTaskSysInfo(void *pvParameters){
	uint8_t pcWriteBuffer[500];
	int temp = 0;
	while(1){
		printf("=================================================\r\n");
		printf("任务名   任务状态   优先级   剩余栈   任务序号\r\n");
		vTaskList((char *)&pcWriteBuffer);
		printf("%s\r\n", pcWriteBuffer);
//		printf("\r\n任务名   运行计数   使用率\r\n");
//		vTaskGetRunTimeStats((char *)&pcWriteBuffer);
//		printf("%s\r\n", pcWriteBuffer);
		
		vTaskDelay(20);
	}
}


static void vTaskLVGL(void *pvParameters){
	page_manager_init(NULL);//传入lv_scr_act()会导致页面成为屏幕的子类，互相之间并不独立
	while(1){
		lv_timer_handler();
		vTaskDelay(5);
	}
}
