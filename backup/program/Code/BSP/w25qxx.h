#ifndef __W25QXX_H__
#define __W25QXX_H__

#include "main.h"

#define W25Q_CS_PORT		GPIOA
#define W25Q_CS_PIN 	GPIO_PIN_15
#define W25Q_CLK_PORT		GPIOB
#define W25Q_CLK_PIN 	GPIO_PIN_3
#define W25Q_MISO_PORT		GPIOB
#define W25Q_MISO_PIN 	GPIO_PIN_4
#define W25Q_MOSI_PORT		GPIOB
#define W25Q_MOSI_PIN 	GPIO_PIN_5

#define W25Q_CS_SET(sta)	HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, sta)


#define W25QXX_PageSize              256
#define W25QXX_PerWritePageSize      256

/*命令*/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		    0x05 
#define W25X_WriteStatusReg		  0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	  0xAB 
#define W25X_DeviceID			        0xAB 
#define W25X_ManufactDeviceID   	0x90 
#define W25X_JedecDeviceID		    0x9F 

#define WIP_Flag                  0x01  /* Write In Progress (WIP) flag */
#define Dummy_Byte                0xFF

/*等待超时时间*/
#define W25Q_FLAG_TIMEOUT         ((uint32_t)0x1000)

uint8_t W25QXX_ReadSR(void);
void W25QXX_Wait_BUSY(void);

uint8_t W25QXX_SectorErase(uint32_t sector_addr);
void W25QXX_BulkErase(void);
void W25QXX_EraseChip(void);
void W25QXX_BufferRead(uint8_t* pbuf, uint32_t read_addr, uint32_t numbyte);
void W25QXX_PageWrite(uint8_t* pbuf, uint32_t write_addr, uint32_t numbyte);
void W25QXX_BufferWrite(uint8_t* pbuf, uint32_t write_addr, uint32_t numbyte);
uint32_t W25QXX_ReadID(void);
void W25QXX_Init(void);

#endif 

