#ifndef __SVR_OTA_H
#define __SVR_OTA_H

#include <stdint.h>

// 公共区信息结构体
#pragma pack(push, 1)
typedef struct _shared_info{
	uint32_t cur_app_size;     // 当前运行程序的大小
	uint8_t update_flag;
	uint8_t error_flag;
}shared_info_t;
#pragma pack(pop)
#define SIZE_VALUE_BYTES	4

void svr_ota_write_shared_info(shared_info_t* info);
void svr_ota_read_shared_info(shared_info_t* info);
void svr_ota_clear_errorflag(void);
void svr_ota_download_proc(void);

#endif