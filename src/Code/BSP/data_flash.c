#include "stm32f4xx.h"                  // Device header
#include "data_flash.h"
#include "process.h"
#include "log.h"

static const char* TAG = "data_flash";

static uint32_t GetSector(uint32_t Address);


void Flash_Write(uint32_t addr, uint64_t* data, uint8_t data_num){
	HAL_FLASH_Unlock();
	
	FLASH_EraseInitTypeDef EraseInit;
	uint32_t SectorError;
	EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInit.Banks = FLASH_BANK_1;
	EraseInit.NbSectors = 1;
	EraseInit.Sector = GetSector(ADDR_FLASH_SECTOR_7);
	EraseInit.VoltageRange =FLASH_VOLTAGE_RANGE_3;
	if(HAL_FLASHEx_Erase(&EraseInit, &SectorError)==HAL_OK){
		;
	}
	
	for(int i=0; i<data_num; i++){
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data[i]);
		addr += 4;
	}
	HAL_FLASH_Lock();
}

void data_flash_write(void){
	LOG_INFO(TAG, "data_flash_write");
	uint64_t param_index[11] = {(uint64_t)password[0], (uint64_t)password[1], (uint64_t)password[2], 
								(uint64_t)password[3], (uint64_t)password[4], (uint64_t)password[5],
								(uint64_t)password[6], (uint64_t)password[7], (uint64_t)password_len,
								(uint64_t)store_cnt, (uint64_t)active_start_addr};
	Flash_Write(ADDR_FLASH_SECTOR_7, param_index, 11);
}

void data_flash_renew(void){
	uint8_t* param_index_8b[1] = {&password_len};
	uint16_t* param_index_16b[1] = {&store_cnt};
	uint32_t* param_index_32b[1] = {&active_start_addr};
	uint32_t addr_temp = ADDR_FLASH_SECTOR_7;
	
	for(int i=0; i<PASSWORD_MAX_LEN; i++){
		password[i] = (uint8_t)(*(uint32_t*)addr_temp);
		addr_temp += 4;
	}
	*param_index_8b[0] = (uint8_t)(*(uint32_t*)addr_temp);
	addr_temp += 4;
	*param_index_16b[0] = (uint16_t)(*(uint32_t*)addr_temp);
	addr_temp += 4;
	*param_index_32b[0] = (uint32_t)(*(uint32_t*)addr_temp);
	addr_temp += 4;
}


static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_SECTOR_7;  
  }
  return sector;
}