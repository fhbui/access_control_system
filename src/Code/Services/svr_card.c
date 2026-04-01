#include "svr_card.h"
#include "rc522.h"
#include "w25qxx.h"
#include <string.h>
#include "log.h"

#define _CARD_SECTOR_1        0x0000
#define _CARD_SECTOR_2        0x1000
#define _CARD_SECTOR_SIZE     0x1000
#define _CARD_SECTOR_NUM      1
#define _INFO_SECTOR          0x4000

static char* TAG = "svr_card";
static uint32_t active_start_addr = _CARD_SECTOR_1;

// 卡驱动面向对象封装
static struct{
    void (*init)(void);
    rc522_status_t (*get_id)(uint8_t* uid);
    rc522_status_t (*select_card)(uint8_t *uid);
    void (*halt)(void);
}driver = {
    .init = rc522_init,
    .get_id = rc522_identify_card,
    .select_card = rc522_select_card,
    .halt = rc522_halt,
};

// 存储设备面向对象封装
static struct {
    uint8_t (*erase_sector)(uint32_t sector_addr, uint32_t num);
    void (*read)(uint32_t read_addr, uint8_t* pbuf, uint32_t numbyte);
    void (*write)(uint32_t write_addr, uint8_t* pbuf, uint32_t numbyte);
}storage_dev = {
    .erase_sector = w25qxx_erase_sector,
    .read = w25qxx_buffer_read,
    .write = w25qxx_buffer_write,
};

// 存储内容结构体
typedef struct  __attribute__((packed)){    // 强制取消所有可能的优化对齐
    uint8_t status;    // 状态标记:0xFF（空白）、0xAA（有效）、0x55（无效/已删除）
    uint8_t card_id[4];  // 4字节卡号
} card_storage_t;

/**
 * @brief 初始化卡服务
 */
void svr_card_init(void){
    driver.init();
    storage_dev.read(_INFO_SECTOR, (uint8_t*)&active_start_addr, 4);
    if(active_start_addr != _CARD_SECTOR_1 && active_start_addr != _CARD_SECTOR_2){
        LOG_ERROR(TAG, "start addr info error");
        active_start_addr = _CARD_SECTOR_1;
        storage_dev.erase_sector(_INFO_SECTOR, 1);
        storage_dev.write(_INFO_SECTOR, (uint8_t*)&active_start_addr, 4);
    }
}

/**
 * @brief 寻卡并获取UID
 * @param uid 存储获取到的4字节UID
 * @return svr_card_status_t 
 */
svr_card_status_t svr_card_get_id(uint8_t* uid){
    rc522_status_t res = driver.get_id(uid);
    if(res == RC522_OK){
		LOG_DEBUG(TAG, "uid is %d %d %d %d", uid[0], uid[1], uid[2], uid[3]);
        driver.select_card(uid);
        driver.halt();
        return SVR_CARD_OK;
    }
	
    else if(res == RC522_NOTAG){
        return SVR_CARD_FAIL;
    }
    else{
		LOG_DEBUG(TAG, "get_id failed");
        return SVR_CARD_FAIL;
    }
}

/**
 * @brief  卡片存储扇区整理与垃圾回收
 * @note   该函数解决了逻辑删除导致的存储空间碎片化问题，通过“对冲切换”确保始终有一个干净的存储空间。
 */
static void _card_sector_clean(void){
    uint32_t read_addr = active_start_addr, write_addr;
    if(read_addr == _CARD_SECTOR_1)         write_addr = _CARD_SECTOR_2;
    else if(read_addr == _CARD_SECTOR_2)    write_addr = _CARD_SECTOR_1;
    
    uint32_t new_addr = write_addr;
    storage_dev.erase_sector(new_addr, _CARD_SECTOR_NUM);

    uint8_t status;
    uint8_t read_id[4];
    storage_dev.read(read_addr, &status, 1);
    while(status != 0xFF && read_addr-active_start_addr < _CARD_SECTOR_SIZE){
        if(status == 0xAA){
            storage_dev.read(read_addr+1, read_id, 4);
            storage_dev.write(write_addr, &status, 1);
            storage_dev.write(write_addr+1, read_id, 4);
            write_addr += sizeof(card_storage_t);
        }
        read_addr += sizeof(card_storage_t);
        storage_dev.read(read_addr, &status, 1);
    }

    storage_dev.erase_sector(active_start_addr, _CARD_SECTOR_NUM);
    active_start_addr = new_addr;
	LOG_DEBUG(TAG, "new card sector is 0x%08x", active_start_addr);
    storage_dev.erase_sector(_INFO_SECTOR, 1);
    storage_dev.write(_INFO_SECTOR, (uint8_t*)&active_start_addr, 4);
}

/**
 * @brief  检查指定 UID 的卡片是否存在于存储中
 * @param  uid: 指向要查找的 4 字节卡片唯一标识符的指针
 * @param  addr_out: [输出参数] 如果找到卡片，存储该卡片在 Flash 中的起始绝对地址；
 *                   若不需要地址可传入 NULL
 */
svr_card_status_t svr_card_exist(uint8_t* uid, uint32_t* addr_out){
    uint32_t search_addr = active_start_addr;
    uint8_t status;
    uint8_t read_id[4];

    storage_dev.read(search_addr, &status, 1);
    while(status != 0xFF && search_addr-active_start_addr < _CARD_SECTOR_SIZE){
        if(status == 0xAA){
            storage_dev.read(search_addr+1, read_id, 4);
            if(memcmp(read_id, uid, 4) == 0){
				LOG_DEBUG(TAG, "card exist in addr: 0x%08x", search_addr);
                if(addr_out != NULL){
                    *addr_out = search_addr;
                }
                return SVR_CARD_OK;
            }
        }
        search_addr += sizeof(card_storage_t);
        storage_dev.read(search_addr, &status, 1);
    }
    return SVR_CARD_FAIL;
}

/**
 * @brief  将新的卡片 UID 保存到存储设备中
 * @param  uid: 指向待保存的 4 字节卡片唯一标识符的指针
 */
svr_card_status_t svr_card_save(uint8_t* uid){
    uint32_t search_addr = active_start_addr; 
    if(svr_card_exist(uid, &search_addr) == SVR_CARD_OK){
        return SVR_CARD_FAIL;
    }

    uint8_t status;
    uint8_t read_id[4];
    storage_dev.read(search_addr, &status, 1);
    while(status != 0xFF && search_addr-active_start_addr < _CARD_SECTOR_SIZE){
        search_addr += sizeof(card_storage_t);
        // 添加判断是否填满，触发回收
        if(search_addr - active_start_addr >= _CARD_SECTOR_SIZE){
			LOG_DEBUG(TAG, "card sector has be filled");
            _card_sector_clean();
            search_addr = active_start_addr;
        }
        storage_dev.read(search_addr, &status, 1);
    }
	
	if(search_addr-active_start_addr < _CARD_SECTOR_SIZE){
		LOG_DEBUG(TAG, "card save to addr: 0x%08x", search_addr);
		uint8_t temp = 0xAA;
		storage_dev.write(search_addr, &temp, 1);
		storage_dev.write(search_addr+1, uid, 4);
		return SVR_CARD_OK;
	}
	LOG_DEBUG(TAG, "card save error");
	return SVR_CARD_FAIL;
}

/**
 * @brief  从存储中逻辑删除指定的卡片
 * @param  uid: 指向要删除的 4 字节卡片唯一标识符的指针
 */
svr_card_status_t svr_card_del(uint8_t* uid){
    uint32_t del_addr;
    if(svr_card_exist(uid, &del_addr) == SVR_CARD_OK){
        uint8_t temp = 0x55;
        storage_dev.write(del_addr, &temp, 1);
        LOG_DEBUG(TAG, "card delete from addr: 0x%08x", del_addr);
        return SVR_CARD_OK;
    }
    return SVR_CARD_FAIL;
}
