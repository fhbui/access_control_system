/**
  ******************************************************************************
  * @file    test.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  */
#include "W25QXX.h"

SPI_HandleTypeDef hspi3;

/************底层读写**********************/

uint8_t W25QXX_SendByte(uint8_t cmd){
	uint8_t rx_data;
	uint8_t tx_data = cmd;
	//CS引脚拉低放在外面写
	HAL_SPI_TransmitReceive(&hspi3, &tx_data, &rx_data, 1, 0x1000);
	return rx_data;
}
	
uint8_t W25QXX_ReceiveByte(void){//感觉确实SPI这样写才好
	return (W25QXX_SendByte(Dummy_Byte));
}

/*****************************************/
uint8_t W25QXX_ReadSR(void){
	uint8_t byte = 0;
	W25Q_CS_SET(0);
	W25QXX_SendByte(W25X_ReadStatusReg);
	byte = W25QXX_ReceiveByte();
	W25Q_CS_SET(1);
	return byte;
}

void W25QXX_WriteEnable(void){
	W25Q_CS_SET(0);
	W25QXX_SendByte(W25X_WriteEnable);
	
	uint32_t time_out = W25Q_FLAG_TIMEOUT;
	while(__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_BSY) && time_out>0){
		time_out--;
	}
	W25Q_CS_SET(1);
}

void W25QXX_WriteDisable(void){
	W25Q_CS_SET(0);
	W25QXX_SendByte(W25X_WriteDisable);
	
	uint32_t time_out = W25Q_FLAG_TIMEOUT;
	while(__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_BSY) && time_out>0){
		time_out--;
	}
	W25Q_CS_SET(1);
}

void W25QXX_Wait_BUSY(void){
	uint8_t ret_flag = 0;
//	while((ret_flag&0x01)==0x01){	//第一次ret_flag = 0就不符合了
//		ret_flag = W25QXX_ReadSR();
//	}
	
	do {
        ret_flag = W25QXX_ReadSR();
    } while((ret_flag & 0x01) == 0x01); // 等待BUSY位为0
}

/*****************************************/

uint8_t W25QXX_SectorErase(uint32_t sector_addr){
	W25QXX_Wait_BUSY();
	W25QXX_WriteEnable();
	
	W25Q_CS_SET(0);
	W25QXX_SendByte(W25X_SectorErase);
	W25QXX_SendByte((sector_addr >> 16)&0xFF);
	W25QXX_SendByte((sector_addr >> 8)&0xFF);
	W25QXX_SendByte(sector_addr & 0xFF);	//24位地址
	
	uint32_t time_out = W25Q_FLAG_TIMEOUT;
	while(__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_BSY) && time_out>0){
		time_out--;
	}
	W25Q_CS_SET(1);
	
	W25QXX_Wait_BUSY();
	W25QXX_WriteDisable();
	
	if(time_out == 0)	return 0;
	else				return 1;
}

void W25QXX_BulkErase(void){
	
}

 /**
  * @brief  读取FLASH数据
  * @param 	pbuf，存储读出数据的指针
  * @param  read_addr，读取地址
  * @param  numbyte，读取数据长度
  * @retval 无
  */
void W25QXX_BufferRead(uint8_t* pbuf, uint32_t read_addr, uint32_t numbyte){
	W25QXX_Wait_BUSY();
	
	W25Q_CS_SET(0);
	W25QXX_SendByte(W25X_ReadData);
	W25QXX_SendByte((read_addr >> 16)&0xFF);
	W25QXX_SendByte((read_addr >> 8)&0xFF);
	W25QXX_SendByte(read_addr & 0xFF);	//24位地址
	
	for(int i=0; i<numbyte; i++){
		*pbuf = W25QXX_ReceiveByte();
		pbuf++;
	}
	
	W25Q_CS_SET(1);
}

 /**
  * @brief  对FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	pBuffer，要写入数据的指针
  * @param WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据长度，必须小于等于W25QXX_PerWritePageSize
  * @retval 无
  */
void W25QXX_PageWrite(uint8_t* pbuf, uint32_t write_addr, uint32_t numbyte){
	W25QXX_Wait_BUSY();
	W25QXX_WriteEnable();
	if(numbyte > W25QXX_PerWritePageSize)	
		numbyte = W25QXX_PerWritePageSize;

	W25Q_CS_SET(0);	
	W25QXX_SendByte(W25X_PageProgram);
	W25QXX_SendByte((write_addr >> 16)&0xFF);
	W25QXX_SendByte((write_addr >> 8)&0xFF);
	W25QXX_SendByte(write_addr & 0xFF);	//24位地址
	
	for(int i=0; i<numbyte; i++){
		W25QXX_SendByte(*pbuf);
		pbuf++;
	}
	
	W25Q_CS_SET(1);	
	W25QXX_Wait_BUSY();
	W25QXX_WriteDisable();
}

 /**
  * @brief  对FLASH写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	pBuffer，要写入数据的指针
  * @param  WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据长度
  * @retval 无
  */	

void W25QXX_BufferWrite(uint8_t* pbuf, uint32_t write_addr, uint32_t numbyte){
    uint32_t bytes_remaining = numbyte;
    uint32_t current_addr = write_addr;
    uint8_t* current_buf = pbuf;

    // 1. 处理首页的非对齐部分（如果存在）
    uint32_t page_offset = current_addr % W25QXX_PageSize;
    if (page_offset != 0) {
        uint32_t bytes_in_first_page = W25QXX_PageSize - page_offset;
        if (bytes_in_first_page > bytes_remaining) {
            bytes_in_first_page = bytes_remaining;
        }
        
        W25QXX_PageWrite(current_buf, current_addr, bytes_in_first_page);
        
        current_buf += bytes_in_first_page;
        current_addr += bytes_in_first_page;
        bytes_remaining -= bytes_in_first_page;
    }

    // 2. 处理完整的中间页
    while (bytes_remaining >= W25QXX_PageSize) {
        W25QXX_PageWrite(current_buf, current_addr, W25QXX_PageSize);
        
        current_buf += W25QXX_PageSize;
        current_addr += W25QXX_PageSize;
        bytes_remaining -= W25QXX_PageSize;
    }

    // 3. 处理最后可能剩下的零碎字节
    if (bytes_remaining > 0) {
        W25QXX_PageWrite(current_buf, current_addr, bytes_remaining);
    }
	
	//比之前的清晰简介好多，野火代码写法又长又难看明白
}

#if 0
void W25QXX_BufferWrite(uint8_t* pbuf, uint32_t write_addr, uint32_t numbyte){
	uint32_t sub_addr = write_addr % W25QXX_PageSize;	//页偏移地址
	uint16_t remain_byte_firstpage = W25QXX_PageSize - sub_addr;	//首页零碎字节数
	uint32_t fullpage_num = numbyte/ W25QXX_PageSize;	//写满的页数
	uint16_t total_fragmented_bytes = numbyte - fullpage_num*W25QXX_PageSize;	//总的零碎字节数
	
	if(remain_byte_firstpage == 0){	//首页无剩余（对齐），直接循环写入
		for(int i=0; i<fullpage_num; i++){
			W25QXX_PageWrite(pbuf, write_addr, W25QXX_PageSize);
			pbuf+=W25QXX_PageSize;
			write_addr+=W25QXX_PageSize;
		}
		if(total_fragmented_bytes > 0){
			W25QXX_PageWrite(pbuf, write_addr, total_fragmented_bytes);
		}
	}
	else{	//首页有剩余字节
		W25QXX_PageWrite(pbuf, write_addr, remain_byte_firstpage);
		pbuf += remain_byte_firstpage;
		write_addr += remain_byte_firstpage;
		
		numbyte -= remain_byte_firstpage;
		fullpage_num = numbyte/ W25QXX_PageSize;	//能被写满的页数
		total_fragmented_bytes = numbyte - fullpage_num*W25QXX_PageSize;	//总的零碎字节数		
		
		for(int i=0; i<fullpage_num; i++){
			W25QXX_PageWrite(pbuf, write_addr, W25QXX_PageSize);
			pbuf+=W25QXX_PageSize;
			write_addr+=W25QXX_PageSize;
		}
		if(total_fragmented_bytes > 0){
			W25QXX_PageWrite(pbuf, write_addr, total_fragmented_bytes);
		}
	}
}
#endif
	
void W25QXX_EraseChip(void){
	W25QXX_Wait_BUSY();
	W25QXX_WriteEnable();
	
	W25Q_CS_SET(0);
	W25QXX_SendByte(W25X_ChipErase);
	W25Q_CS_SET(1);
	
	W25QXX_Wait_BUSY();
	W25QXX_WriteDisable();
}

uint32_t W25QXX_ReadID(void){
	W25Q_CS_SET(0);
	W25QXX_SendByte(0x90);
	W25QXX_SendByte(0x00);
	W25QXX_SendByte(0x00);
	W25QXX_SendByte(0x00);
	uint32_t temp1 = (uint32_t)W25QXX_ReceiveByte();
	uint32_t temp2 = (uint32_t)W25QXX_ReceiveByte();
	W25Q_CS_SET(1);
	
	return (temp1<<8)|(temp2);
	//0XEF14,表示芯片型号为W25Q16   
}

void W25QXX_GPIOInit(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_SPI3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	  
	GPIO_InitStruct.Pin = W25Q_CS_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(W25Q_CS_PORT, &GPIO_InitStruct);	
	
	GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void W25QXX_Init(void){
	W25QXX_GPIOInit();
	
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi3) != HAL_OK)
	{
	Error_Handler();
	}	
}