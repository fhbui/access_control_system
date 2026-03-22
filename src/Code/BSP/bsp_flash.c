#include "main.h"
#include "bsp_flash.h"
#include <string.h>
#include "log.h"

#define FLASH_BASE_ADDR      0x08000000
#define FLASH_USER_START     0x0803F800  // 假设最后 2KB 用于存储参数

// 实际使用可定义结构体来存储、读取

static uint32_t bsp_flash_get_sector(uint32_t Address)
{
    uint32_t sector = 0;

    if ((Address < 0x08004000) && (Address >= 0x08000000)) sector = FLASH_SECTOR_0;  // 16 Kbytes
    else if ((Address < 0x08008000) && (Address >= 0x08004000)) sector = FLASH_SECTOR_1;  // 16 Kbytes
    else if ((Address < 0x0800C000) && (Address >= 0x08008000)) sector = FLASH_SECTOR_2;  // 16 Kbytes
    else if ((Address < 0x08010000) && (Address >= 0x0800C000)) sector = FLASH_SECTOR_3;  // 16 Kbytes
    else if ((Address < 0x08020000) && (Address >= 0x08010000)) sector = FLASH_SECTOR_4;  // 64 Kbytes
    else if ((Address < 0x08040000) && (Address >= 0x08020000)) sector = FLASH_SECTOR_5;  // 128 Kbytes
    else if ((Address < 0x08060000) && (Address >= 0x08040000)) sector = FLASH_SECTOR_6;  // 128 Kbytes
    else if ((Address < 0x08080000) && (Address >= 0x08060000)) sector = FLASH_SECTOR_7;  // 128 Kbytes
    else if ((Address < 0x080A0000) && (Address >= 0x08080000)) sector = FLASH_SECTOR_8;  // 128 Kbytes
    else if ((Address < 0x080C0000) && (Address >= 0x080A0000)) sector = FLASH_SECTOR_9;  // 128 Kbytes
    else if ((Address < 0x080E0000) && (Address >= 0x080C0000)) sector = FLASH_SECTOR_10; // 128 Kbytes
    else if ((Address < 0x08100000) && (Address >= 0x080E0000)) sector = FLASH_SECTOR_11; // 128 Kbytes
    else sector = 0xFFFFFFFF; // 地址越界

    return sector;
}

/**
 * @brief 内部 Flash 写入数据 (自动处理擦除与解锁)
 * @param addr  要写入的起始地址 (建议设置对齐检查)
 * @param pdata 数据源指针
 * @param len   数据长度 (字节)
 */
bsp_flash_status_t bsp_flash_write(uint32_t addr, const uint8_t *pdata, uint32_t len) {
    HAL_StatusTypeDef status;
    uint32_t page_error = 0;

    // 1. 安全检查：禁止操作 APP 代码区
    if (addr < FLASH_USER_START) return BSP_FLASH_ERROR;

    // 获取起始和结束地址对应的扇区号
    uint32_t first_sector = bsp_flash_get_sector(addr);
    uint32_t last_sector  = bsp_flash_get_sector(addr + len - 1); // 注意要减1避免正好压线
    if (first_sector == 0xFFFFFFFF || last_sector == 0xFFFFFFFF) return BSP_FLASH_ERROR;

    // 计算需要擦除的扇区总数
    uint32_t nb_sectors = (last_sector - first_sector) + 1;

    // 2. 解锁 Flash 控制器
    HAL_FLASH_Unlock();

    // 3. 擦除目标扇区
    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.NbSectors = nb_sectors; 
    erase_init.Sector = first_sector;
    erase_init.Banks = FLASH_BANK_1;
    erase_init.VoltageRange =FLASH_VOLTAGE_RANGE_3;

    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return BSP_FLASH_ERROR;
    }

    // 4. 循环写入数据 (处理对齐问题，可能需 64 位双字对齐)
    for (uint32_t i = 0; i < len; i += 8) {
        uint64_t data_chunk = 0;
        memcpy(&data_chunk, pdata + i, (len - i) >= 8 ? 8 : (len - i));
        
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, data_chunk);
        if (status != HAL_OK) break;
    }

    // 5. 锁定 Flash
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? BSP_FLASH_OK : BSP_FLASH_ERROR;
}

/**
 * @brief 内部 Flash 读取 (直接内存访问的包装)
 */
void bsp_flash_read(uint32_t addr, uint8_t *pbuf, uint32_t len) {
    // 内部 Flash 已经映射到地址空间，直接拷贝即可，性能最高
    memcpy(pbuf, (void *)addr, len);
}

