#include "flash.h"
#include <stdio.h>

void flash_erase(uint32_t addr, uint32_t num){
    uint32_t sector;

    if(addr>=0x08000000 && addr<=0x08003FFF)        sector = FLASH_SECTOR_0;
    else if(addr>=0x08004000 && addr<=0x08007FFF)   sector = FLASH_SECTOR_1;
    else if(addr>=0x08008000 && addr<=0x0800BFFF)   sector = FLASH_SECTOR_2;
    else if(addr>=0x0800C000 && addr<=0x0800FFFF)   sector = FLASH_SECTOR_3;
    else if(addr>=0x08010000 && addr<=0x0801FFFF)   sector = FLASH_SECTOR_4;
    else if(addr>=0x08020000 && addr<=0x0803FFFF)   sector = FLASH_SECTOR_5;
    else if(addr>=0x08040000 && addr<=0x0805FFFF)   sector = FLASH_SECTOR_6;
    else if(addr>=0x08060000 && addr<=0x0807FFFF)   sector = FLASH_SECTOR_7;
    else if(addr>=0x08080000 && addr<=0x0809FFFF)   sector = FLASH_SECTOR_8;
    else if(addr>=0x080A0000 && addr<=0x080BFFFF)   sector = FLASH_SECTOR_9;
    else if(addr>=0x080C0000 && addr<=0x080DFFFF)   sector = FLASH_SECTOR_10;
    else if(addr>=0x080E0000 && addr<=0x080FFFFF)   sector = FLASH_SECTOR_11; 

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef p_eraseinit;
    p_eraseinit.TypeErase = FLASH_TYPEERASE_SECTORS;
    p_eraseinit.Sector = sector;
    p_eraseinit.NbSectors = num;
    p_eraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t sector_err = 0;
    HAL_StatusTypeDef res= HAL_FLASHEx_Erase(&p_eraseinit, &sector_err);
	if(res == HAL_ERROR){
		printf("Erase Error at %d\r\n", sector_err);
	}

    HAL_FLASH_Lock();
}

void flash_read(uint32_t addr, uint32_t* buf, uint32_t size){
    for(uint32_t i=0; i<size; i++){
        buf[i] = *(__IO uint32_t* )(addr+4*i);
    }
}

void flash_write(uint32_t addr, uint32_t* buf, uint32_t size){
    HAL_FLASH_Unlock();
	uint32_t temp_val = 0;
    for(uint32_t i=0; i<size; i++){
		temp_val = buf[i];
        HAL_StatusTypeDef res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+4*i, temp_val);
		if(res == HAL_ERROR){
			printf("Write Error\r\n");
		}
    }
    HAL_FLASH_Lock();
}


void flash_read_generic(uint32_t addr, void* buf, uint32_t size){
    uint8_t* p_dst = (uint8_t*)buf;
    __IO uint8_t* p_src = (__IO uint8_t*)addr;
    for(uint32_t i = 0; i < size; i++){
        p_dst[i] = p_src[i];
    }
}

void flash_write_generic(uint32_t addr, void* buf, uint32_t size){
    uint8_t* p_src = (uint8_t*)buf;
	
    HAL_FLASH_Unlock();
    for(uint32_t i=0; i<size; i++){
        HAL_StatusTypeDef res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr+i, (uint64_t)p_src[i]);
		if(res == HAL_ERROR){
			printf("Write Error\r\n");
		}
    }
    HAL_FLASH_Lock();
}

void flash_test(void){
    uint32_t buf[5] = {1,2,3,4,5};
    uint32_t read_buf[10] = {0};
    flash_erase(0x080C0000, 2);
    flash_write(0x080C0000, buf, 5);
    flash_write(0x080E0000, buf, 5);
	flash_compare(0x080C0000, 0x080E0000, 5);
	
    flash_read(0x080C0000, read_buf, 5);
    flash_read(0x080E0000, &read_buf[5], 5);

	printf("\r\n");
    for(int i=0; i<10; i++){
        printf("%d ", read_buf[i]);
    }
    printf("\r\n");
    
    flash_erase(0x080C0000, 2);
    flash_read(0x080C0000, read_buf, 5);
    flash_read(0x080E0000, &read_buf[5], 5);

    for(int i=0; i<10; i++){
        printf("%d ", read_buf[i]);
    }
    printf("\r\n");
}

void flash_generic_test(void){
    uint32_t buf[5] = {1,2,3,4,5};
    uint32_t read_buf[10] = {0};
    flash_erase(0x080C0000, 2);
    flash_write_generic(0x080C0000, buf, 5*sizeof(uint32_t));
    flash_write_generic(0x080E0000, buf, 5*sizeof(uint32_t));
	
    flash_read_generic(0x080C0000, read_buf, 5*sizeof(uint32_t));
    flash_read_generic(0x080E0000, &read_buf[5], 5*sizeof(uint32_t));

    for(int i=0; i<10; i++){
        printf("%d ", read_buf[i]);
    }
    printf("\r\n");
    
    flash_erase(0x080C0000, 2);
    flash_read_generic(0x080C0000, read_buf, 5*sizeof(uint32_t));
    flash_read_generic(0x080E0000, &read_buf[5], 5*sizeof(uint32_t));

    for(int i=0; i<10; i++){
        printf("%d ", read_buf[i]);
    }
    printf("\r\n");	
}

void flash_compare(uint32_t addr1, uint32_t addr2, uint16_t size){

    for(int i=0; i<size; i++){
        uint32_t temp1 = *(__IO uint32_t* )(addr1+4*i);
        uint32_t temp2 = *(__IO uint32_t* )(addr2+4*i);
        if(temp1 != temp2){
			printf("Not Same at %d\r\n", i);
        }
    }
}

void flash_compare_to_buf(uint32_t addr, uint32_t* buf, uint16_t size){
	uint32_t temp = 0;
    for(int i=0; i<size; i++){
        temp = *(__IO uint32_t * )(addr+4*i);
        if(temp != buf[i]){
//            printf("Not Same at 0x%x\r\n", i);
			printf("[ERROR] %d times: Not same at 0x%x, addr data is 0x%x, buf data is 0x%x\r\n", i, (addr+4*i), temp, buf[i]);
        }else{
			//printf("%d times: Same at 0x%x, addr data is 0x%x, buf data is 0x%x\r\n", i, (addr+4*i), temp, buf[i]);
		}
    }
}

void flash_check_empty(uint32_t addr, uint16_t size){
    for(int i=0; i<size; i++){
        uint8_t temp1 = *(__IO uint8_t* )(addr+i);
        if(temp1 != 0xFF){
            printf("Not empty\r\n");
            return ;
        }
    }
}
