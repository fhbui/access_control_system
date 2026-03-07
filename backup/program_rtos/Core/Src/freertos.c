/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dwt.h"
#include "process.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "my_ui.h"
#include "as608.h"
#include "RC522.h"
#include "lcd.h"
#include "data_flash.h"
#include "tim.h"
#include "log.h"

#include "semphr.h"
#include "queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static const char* TAG = "freertos";

static TaskHandle_t xHandleSysInfo;
static TaskHandle_t xHandle_TaskPCD;
static TaskHandle_t xHandle_TaskFP;
static TaskHandle_t xHandle_TaskLVGL;

SemaphoreHandle_t xMutex_Flash;		//访问flash互斥量
SemaphoreHandle_t xSemaphore_FPflag;	//和中断通信的信号量

QueueHandle_t xQueue_PCDFlag;
QueueHandle_t xQueue_FPFlag;
QueueHandle_t queue_msgbox_info;
// QueueHandle_t xQueue_State;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

static void vTaskSysInfo(void *pvParameters);
static void vTask_LEDTest(void *pvParameters);

static void vTask_Door(void *pvParameters);
static void vTask_PCD(void *pvParameters);
static void vTask_FP(void *pvParameters);
static void vTask_LVGL(void *pvParameters);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
	xMutex_Flash = xSemaphoreCreateMutex();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	xSemaphore_FPflag = xSemaphoreCreateBinary();
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	xQueue_PCDFlag = xQueueCreate(5, sizeof(pcd_flag_t));
	xQueue_FPFlag = xQueueCreate(5, sizeof(fp_flag_t));
  queue_msgbox_info = xQueueCreate(5, sizeof(ui_msgbox_info_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
//  xTaskCreate(vTask_LEDTest, "LEDTest", 64, NULL, 2, NULL);
//  xTaskCreate(vTaskSysInfo, "SysInfo", 256, NULL, 2, &xHandleSysInfo);	//128太小，直接HardFault
  xTaskCreate(vTask_PCD, "PCD_Task", 128, NULL, 3, &xHandle_TaskPCD);
  xTaskCreate(vTask_FP, "FP_Task", 128, NULL, 3, &xHandle_TaskFP);
  xTaskCreate(vTask_LVGL, "LVGL_Task", 512, NULL, 3, &xHandle_TaskLVGL);
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
  * @brief  LED测试函数
  * @retval 无
  */
static void vTask_LEDTest(void *pvParameters){
	while(1){
		bsp_Delayms(1000);
		LED_ON;
		bsp_Delayms(1000);
		LED_OFF;
	}
}

/**
  * @brief  OS系统运行信息打印
  * @retval 无
  */
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
		
		vTaskDelay(1000);
	}
}

/**
  * @brief  门任务函数
  * @retval 无
  */
static void vTask_Door(void *pvParameters){
	printf("The door is opened\r\n");
}

/**
  * @brief  M1卡识别处理任务函数
  * @retval 无
  */
static void vTask_PCD(void *pvParameters){
  taskENTER_CRITICAL();
  RC522_Init();
  taskEXIT_CRITICAL();
	RC522_Rese();	// 函数里面用到HAL_Delay的别放进临界段
	RC522_Config_Type();  // 设置RC522的工作方式
	pcd_flag_t flag = PCD_CHECK_EXIST;

	while(1){
		if(xQueueReceive(xQueue_PCDFlag, &flag, 10) == pdPASS){
      LOG_DEBUG(TAG, "pcd_flag is %d", flag);
    }
    pcd_scan(flag);
    vTaskDelay(50);
	}
}

/**
  * @brief  指纹识别处理任务函数
  * @retval 无
  */
static void vTask_FP(void *pvParameters){
  taskENTER_CRITICAL();
  as608_init();
  taskEXIT_CRITICAL();
	fp_flag_t flag = FP_VERIFY;
  uint8_t ret = 0;

	while(1){
    BaseType_t res;
    if(ret == 1 && (flag==FP_ADD)){
      // 完成add、delete后一直等待UI切换到主页
      res = xQueueReceive(xQueue_FPFlag, &flag, portMAX_DELAY);
    }
    else{
		  res = xQueueReceive(xQueue_FPFlag, &flag, 10);
    }
    if(res == pdPASS){
      LOG_DEBUG(TAG, "fp_flag is %d", flag);
    }
    
		ret = fp_scan(flag);
		vTaskDelay(50);
	}
}

/**
  * @brief  UI显示处理任务函数
  * @retval 无
  */
static void vTask_LVGL(void *pvParameters){
  taskENTER_CRITICAL();
  lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	my_ui_init();
	MX_TIM7_Init();		//触屏周期中断
  taskEXIT_CRITICAL();
	while(1){
			lv_timer_handler();
			vTaskDelay(5);
	}
}



/* USER CODE END Application */

