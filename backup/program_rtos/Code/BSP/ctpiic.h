#ifndef __CTPIIC_H
#define __CTPIIC_H
#include "main.h"	    

#define u8 uint8_t
#define u16 uint16_t

/****************引脚配置****************/

#define CTP_SDA_PORT	GPIOB
#define CTP_SDA_PIN		GPIO_PIN_9
#define CTP_SCL_PORT	GPIOB
#define CTP_SCL_PIN		GPIO_PIN_8
  	   		   
//IIC IO方向设置
//#define CTP_SDA_IN()  {GPIOF->MODER&=~(3<<(11*2));GPIOF->MODER|=0<<11*2;}
//#define CTP_SDA_OUT() {GPIOF->MODER&=~(3<<(11*2));GPIOF->MODER|=1<<11*2;}

//IO操作函数	 
#define CTP_IIC_SCL(val)    HAL_GPIO_WritePin(CTP_SCL_PORT, CTP_SCL_PIN, val) 			//SCL     
#define CTP_IIC_SDA(val)    HAL_GPIO_WritePin(CTP_SDA_PORT, CTP_SDA_PIN, val) 			//SDA	 
#define CTP_READ_SDA()   	HAL_GPIO_ReadPin(CTP_SDA_PORT, CTP_SDA_PIN) 	  			//输入SDA 

//IIC所有操作函数
void CTP_IIC_Init(void);                	//初始化IIC的IO口				 
void CTP_IIC_Start(void);				//发送IIC开始信号
void CTP_IIC_Stop(void);	  				//发送IIC停止信号
void CTP_IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 CTP_IIC_Read_Byte(unsigned char ack);	//IIC读取一个字节
u8 CTP_IIC_Wait_Ack(void); 				//IIC等待ACK信号
void CTP_IIC_Ack(void);					//IIC发送ACK信号
void CTP_IIC_NAck(void);					//IIC不发送ACK信号

#endif







