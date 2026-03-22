#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#include "main.h"

typedef enum{
    BSP_FLASH_OK = 0,
    BSP_FLASH_ERROR,
}bsp_flash_status_t;

bsp_flash_status_t bsp_flash_write(uint32_t addr, const uint8_t *pdata, uint32_t len);
void bsp_flash_read(uint32_t addr, uint8_t *pbuf, uint32_t len);

#endif