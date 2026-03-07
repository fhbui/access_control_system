#ifndef __AS608_H
#define __AS608_H
#include <stdio.h>
#include "main.h"

/*指令&格式*/
#define AS608_HEAD	0xEF01		//包头（2bits）
#define AS608_ADDR	0XFFFFFFFF	//芯片地址（4bits）

/*手指状态*/
#define FINGER_NO_EXIST		0x00
#define FINGER_EXIST		0x01

/*缓冲区号*/
#define CharBuffer1		0x01
#define CharBuffer2		0x02

// 引脚接口
#define AS608_WAK_PORT		GPIOD
#define AS608_WAK_PIN		GPIO_PIN_4
#define AS608_TXD_PORT		GPIOD
#define AS608_TXD_PIN		GPIO_PIN_6  // UART2_RX
#define AS608_RXD_PORT	    GPIOD
#define AS608_RXD_PIN		GPIO_PIN_5  // UART2_TX

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern volatile uint8_t receive_flag;
extern volatile uint8_t finger_status;

extern uint16_t id_index;		//ID号索引
extern uint16_t fragmented_num;		//零碎空号数目
extern uint16_t fragmented_index[];

uint8_t as608_detection_data(uint16_t wait_time,uint16_t **ID_OR_NUM);
uint8_t as608_detection_checknum(char *data , uint16_t **ID_OR_NUM);
uint8_t as608_detection_finger(uint16_t wait_time);
uint8_t as608_add_fingerprint(uint16_t PageID);
uint16_t as608_verify_fingerprint(uint16_t* id_val);
uint8_t as608_delete_fingerprint(void);
uint8_t as608_empty_all_fingerprint(void);
uint16_t as608_find_fingerprints_num(void);

uint8_t as608_check_idle_id(uint16_t PageID);

void as608_init(void);
void HAL_UART_RxIdleCallback(UART_HandleTypeDef *huart);
#endif

