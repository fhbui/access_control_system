#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "main.h"

/*************引脚分配*****************/
#define TOUCH_INT_PORT		GPIOA
#define TOUCH_INT_PIN		GPIO_PIN_4
#define TOUCH_MISO_PORT		GPIOA
#define TOUCH_MISO_PIN		GPIO_PIN_6
#define TOUCH_MOSI_PORT		GPIOA
#define TOUCH_MOSI_PIN		GPIO_PIN_7
#define TOUCH_SCLK_PORT		GPIOA
#define TOUCH_SCLK_PIN		GPIO_PIN_5
#define TOUCH_CS_PORT		GPIOA
#define TOUCH_CS_PIN		GPIO_PIN_3

#define INT_PIN_STATE		HAL_GPIO_ReadPin(TOUCH_INT_PORT, TOUCH_INT_PIN)	
#define MISO_PIN_STATE		HAL_GPIO_ReadPin(TOUCH_MISO_PORT, TOUCH_MISO_PIN)

#define CS_PIN_SET(sta)		HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, sta)
#define MOSI_PIN_SET(sta)	HAL_GPIO_WritePin(TOUCH_MOSI_PORT, TOUCH_MOSI_PIN, sta)
#define SCLK_PIN_SET(sta)	HAL_GPIO_WritePin(TOUCH_SCLK_PORT, TOUCH_SCLK_PIN, sta)

/* 控制字 */
#define CMD_RDY 0X90  //0B10010000即用差分方式读X坐标
#define CMD_RDX	0XD0  //0B11010000即用差分方式读Y坐标  

// 这些命令可以返回固定的测试值
#define XPT_CMD_TEST_1      0xE0  // 1110 0000: 测试模式1
#define XPT_CMD_TEST_2      0xF0  // 1111 0000: 测试模式2

/* 触屏状态	*/ 
#define Key_Down 0x01
#define Key_Up   0x00 

/* 笔杆结构体 */
typedef struct 
{
	uint16_t X0;//原始坐标
	uint16_t Y0;
	uint16_t X; //最终/暂存坐标
	uint16_t Y;						   	    
	uint8_t  Key_Sta;//笔的状态			  
	//触摸屏校准参数
	float xfac;
	float yfac;
	short xoff;
	short yoff;
}Pen_Holder;

/* 触摸状态枚举 */
typedef enum {
    TOUCH_IDLE = 0,     // 空闲状态
    TOUCH_PRESSED,      // 按下状态
    TOUCH_HELD,         // 长按状态
    TOUCH_RELEASED      // 释放状态
} Touch_State;

extern Pen_Holder Pen_Point;

uint16_t XPT_Read_AD(uint8_t CMD);
uint16_t XPT_Read_XY(uint8_t xy_cmd);
uint8_t XPT_Read(uint16_t *x,uint16_t *y);
void Pen_Int_Set(uint8_t en_val);
uint8_t Read_TP_Once(void);
void Convert_Pos(void);
void Touch_Init();
//void Touch_Test();

void Touch_Falling_PIT(void);
void Touch_Process(void);

#endif 

