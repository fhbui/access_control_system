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
#include "ui_main.h"
#include "log.h"
#include "app_interface.h"

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

SemaphoreHandle_t mutex_flash;

QueueHandle_t ui_msg_queue;

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

static void task_door_control(void *pvParameters);
static void task_card_proc(void *pvParameters);
static void task_finger_proc(void *pvParameters);
static void task_ui_proc(void *pvParameters);

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
	mutex_flash = xSemaphoreCreateMutex();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  ui_msg_queue = xQueueCreate(5, sizeof(interface_ui_msg_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
//  xTaskCreate(vTask_LEDTest, "LEDTest", 64, NULL, 2, NULL);
//  xTaskCreate(vTaskSysInfo, "SysInfo", 256, NULL, 2, &xHandleSysInfo);
  xTaskCreate(task_card_proc, "PCD_Task", 128, NULL, 3, &xHandle_TaskPCD);
  xTaskCreate(task_finger_proc, "FP_Task", 128, NULL, 3, &xHandle_TaskFP);
  xTaskCreate(task_ui_proc, "LVGL_Task", 512, NULL, 3, &xHandle_TaskLVGL);
  
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

static void vTask_LEDTest(void *pvParameters){
	while(1){
		HAL_Delay(1000);
		LED_ON;
		HAL_Delay(1000);
		LED_OFF;
	}
}

static void vTaskSysInfo(void *pvParameters){
	uint8_t pcWriteBuffer[500];
	int temp = 0;
	while(1){
		printf("=================================================\r\n");
		vTaskList((char *)&pcWriteBuffer);
		printf("%s\r\n", pcWriteBuffer);
		vTaskDelay(1000);
	}
}

static void vTask_Door(void *pvParameters){
	printf("The door is opened\r\n");
}

static void task_card_proc(void *pvParameters){

}

static void task_finger_proc(void *pvParameters){

}

static void task_ui_proc(void *pvParameters){

}

/* USER CODE END Application */

