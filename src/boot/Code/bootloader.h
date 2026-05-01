#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "main.h"

#define SHARED_ADDR         0x0800C000      // Sector 3
#define APP_ADDR    0x08020000      // Sector 5~6
#define BACKUP_ADDR    0x08060000      // Sector 7~8
#define TEMP_ADDR   0x080A0000      // Sector 9
#define APP_SIZE    	0x80000

int bootloader_checkupdate(void);
uint32_t bootloader_get_active_addr(void);
uint32_t bootloader_get_backup_addr(void);
void bootloader_set_updateflag(uint8_t val);
void bootloader_set_errorflag(uint8_t val);

void bootloader_executeapp(void);
void bootloader_updateapp(void);
void bootloader_init(void);

#endif
