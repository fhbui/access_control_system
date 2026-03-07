/**
  ******************************************************************************
  * @file    touch.c
  * @brief   显示屏触摸驱动（IC型号:xpt2046）
  ******************************************************************************
  */
#include "lcd.h"
#include "touch.h"
#include "dwt.h"
#include <math.h>
#include "stdlib.h"

_m_tp_dev tp_dev=
{
	TP_Init,
	NULL,
	0,
	0,
 	0,	
};
//默认为touchtype=0的数据.
 
/*****************************************************************************
 * @name       :u8 TP_Init(void)
 * @date       :2018-08-09 
 * @function   :Initialization touch screen
 * @parameters :None
 * @retvalue   :0-no calibration
								1-Has been calibrated
******************************************************************************/  
u8 TP_Init(void)
{			    		   
	if(FT6336_Init())
	{
		return 1;
	}
	tp_dev.scan=FT6336_Scan;	//扫描函数指向GT911触摸屏扫描
	return 0;
}
