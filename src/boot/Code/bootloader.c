#include "bootloader.h"
#include <stdio.h>
#include "log.h"

#pragma pack(push, 1)
typedef struct _shared_info{
	uint32_t cur_app_size;
	uint8_t update_flag;
	uint8_t error_flag;
}shared_info_t;
#pragma pack(pop)
#define SIZE_VALUE_BYTES	4

static shared_info_t shared_info;

//====================== internal flash =====================================
#include "flash.h"
#define flash_write_shared_info()		flash_erase(SHARED_ADDR, 1);			\
										flash_write_generic(SHARED_ADDR, &shared_info, 1*sizeof(shared_info_t))
#define flash_read_shared_info()		flash_read_generic(SHARED_ADDR, &shared_info, 1*sizeof(shared_info_t))

#define erase_internal_flash(addr, num)				flash_erase(addr, num)
#define read_internal_flash(addr, pdata, size)		flash_read_generic(addr, pdata, size)
#define write_internal_flash(addr, pdata, size)		flash_write_generic(addr, pdata, size)
//====================== external flash =====================================
#define erase_external_flash(addr, num)				flash_erase(addr, num)
#define read_external_flash(addr, pdata, size)		flash_read_generic(addr, pdata, size)		
#define write_external_flash(addr, pdata, size)		flash_write_generic(addr, pdata, size)
//===========================================================================
	
static const char* TAG = "bootloader";
										
int bootloader_checkupdate(void){
	return shared_info.update_flag;
}

void bootloader_set_updateflag(uint8_t val){
	flash_read_shared_info();
	shared_info.update_flag = val;
	flash_write_shared_info();
}

void bootloader_set_errorflag(uint8_t val){
	flash_read_shared_info();
	shared_info.error_flag = val;
	flash_write_shared_info();	
}

typedef enum{
	INTERNAL_TO_EXTERNAL = 0,
	EXTERNAL_TO_INTERNAL,
	EXTERNAL_TO_EXTERNAL
}direction_t;

void flash_datum_exchange(direction_t dir, uint32_t source_addr, uint32_t target_addr, uint32_t size){
	uint8_t buf[128];
	
	// erase the target section
	if(dir == INTERNAL_TO_EXTERNAL || dir == EXTERNAL_TO_EXTERNAL){
		erase_external_flash(target_addr, 2);
	}
	else if(dir == EXTERNAL_TO_INTERNAL){
		erase_internal_flash(target_addr, 2);
	}
	
	int for_temp = size/128;
	for(int i=0; i<for_temp; i++){
		if(dir==INTERNAL_TO_EXTERNAL){
			read_internal_flash(source_addr+128*i, buf, 128);
			write_external_flash(target_addr+128*i, buf, 128);
		}
		else if(dir==EXTERNAL_TO_EXTERNAL){
			read_external_flash(source_addr+128*i, buf, 128);
			write_external_flash(target_addr+128*i, buf, 128);
		}
		else if(dir==EXTERNAL_TO_INTERNAL){
			read_external_flash(source_addr+128*i, buf, 128);
			write_internal_flash(target_addr+128*i, buf, 128);
		}
		LOG_DEBUG(TAG, "read from 0x%08x, write to 0x%08x", source_addr+128*i, target_addr+128*i);
	}
	
	int remain = size%128;
	if(remain){
		if(dir==INTERNAL_TO_EXTERNAL){
			read_internal_flash(source_addr+128*for_temp, buf, remain);
			write_external_flash(target_addr+128*for_temp, buf, remain);
		}
		else if(dir==EXTERNAL_TO_EXTERNAL){
			read_external_flash(source_addr+128*for_temp, buf, remain);
			write_external_flash(target_addr+128*for_temp, buf, remain);
		}
		else if(dir==EXTERNAL_TO_INTERNAL){
			read_external_flash(source_addr+128*for_temp, buf, remain);
			write_internal_flash(target_addr+128*for_temp, buf, remain);
		}
		LOG_DEBUG(TAG, "read from 0x%08x, write to 0x%08x", source_addr+128*for_temp, target_addr+128*for_temp);
	}
}

// execute update function
void bootloader_updateapp(void){	
	uint32_t new_size;
	uint8_t no_backup_flag = 0;
	
	// 一定要检查各种大小变量：
	// 1. 存储的当前固件大小过大，依旧备份
	// 2. 存储的新固件大小过大，不升级
	read_external_flash(BACKUP_ADDR, (uint8_t*)&new_size, SIZE_VALUE_BYTES);
	if(new_size > APP_SIZE){
		shared_info.update_flag = 0;
		flash_write_shared_info();
		LOG_ERROR(TAG, "new size is wrong: %d", new_size);
		return ;
	}
	if(shared_info.cur_app_size > APP_SIZE){
		shared_info.cur_app_size = APP_SIZE;
	}
	
	// store to TEMP_ADDR
	LOG_DEBUG(TAG, "store to TEMP_ADDR");
	flash_datum_exchange(INTERNAL_TO_EXTERNAL, APP_ADDR, TEMP_ADDR, shared_info.cur_app_size);
	
	// update the APP_ADDR
	LOG_DEBUG(TAG, "update the APP_ADDR");
	flash_datum_exchange(EXTERNAL_TO_INTERNAL, BACKUP_ADDR+SIZE_VALUE_BYTES, APP_ADDR, new_size);
	
	// execute backup
	LOG_DEBUG(TAG, "execute backup");
	uint32_t old_size = shared_info.cur_app_size;
	LOG_INFO(TAG, "old size is %d", old_size);
	shared_info.cur_app_size = new_size;
	flash_datum_exchange(EXTERNAL_TO_EXTERNAL, TEMP_ADDR, BACKUP_ADDR+SIZE_VALUE_BYTES, old_size);
	write_external_flash(BACKUP_ADDR, (uint8_t*)&old_size, SIZE_VALUE_BYTES);	// 放在交换的后面，因为交换内含清除扇区
	
	// set shared flags
	shared_info.update_flag = 0;
	shared_info.error_flag = 1;		// reset by new application
	flash_write_shared_info();
}


__asm void MSR_MSP(uint32_t addr){
	MSR MSP, r0
	BX  r14
}

typedef void (*pc_adjust)(void);

void bootloader_executeapp(void){
	__disable_irq();
	pc_adjust pc;
	uint32_t addr = APP_ADDR;
	if(( (*(uint32_t *) addr) & 0x2FFE0000 ) == 0x20000000){
		MSR_MSP(*(uint32_t *) addr);
		pc = (pc_adjust)(*(uint32_t*)(addr+4));
		pc();
	}
}

void bootloader_init(void){
	flash_read_shared_info();
	
	if(shared_info.error_flag){
		// transfer old firmware data (backup to app)
		uint32_t size = 0;
		read_external_flash(BACKUP_ADDR, (uint8_t*)&size, sizeof(uint32_t));
		if(size >= APP_SIZE){
			LOG_ERROR(TAG, "Error and old size is wrong");
		}
		else{
			flash_datum_exchange(EXTERNAL_TO_INTERNAL, BACKUP_ADDR+4, APP_ADDR, size);
			shared_info.cur_app_size = size;
			LOG_ERROR(TAG, "Error, return to old version!");
		}
		shared_info.error_flag = 0;
		flash_write_shared_info();
	}
}
