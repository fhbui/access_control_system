#ifndef __BSP_DWT_H
#define __BSP_DWT_H
#include "stm32f4xx.h"                  // Device header

void bsp_InitDWT(void);
void bsp_Delayus(uint32_t _ulDelayTime);
void bsp_Delayms(uint32_t _ulDelayTime);

#endif
