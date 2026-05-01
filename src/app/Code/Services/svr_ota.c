#include "svr_ota.h"
#include "bsp_flash.h"
#include "bsp_uart.h"
#include "w25qxx.h"
#include "aes128.h"
#include "hmac.h"
#include "ota_protocol.h"
#include "log.h"
#include <string.h>

static const char* TAG = "svr_ota";

// 分区信息定义
#define SHARED_ADDR         0x0800C000      // Sector 3
#define SECTION_APP_ADDR    0x08020000      // Sector 5~6
#define SECTION_BKU_ADDR    0x08060000      // Sector 7~8
#define SECTION_TEMP_ADDR   0x080A0000      // Sector 9
#define SECTION_SIZE    0x40000     // 256KB
#define SECTION_NUM     2             

// flash设备操作对象
#if 1
static struct{
    bsp_flash_status_t (*erase_sector)(uint32_t addr, uint32_t num);
    bsp_flash_status_t (*write)(uint32_t addr, const uint8_t *pdata, uint32_t len);
    void (*read)(uint32_t addr, uint8_t *pbuf, uint32_t len);
}flash_dev = {
    .erase_sector = bsp_flash_erase_sector,
    .write = bsp_flash_write,
    .read = bsp_flash_read,
};
#else
static struct {
    uint8_t (*erase_sector)(uint32_t addr, uint32_t num);
    void (*read)(uint32_t addr, uint8_t* pbuf, uint32_t numbyte);
    void (*write)(uint32_t addr, uint8_t* pbuf, uint32_t numbyte);
}flash_dev = {
    .erase_sector = w25qxx_erase_sector,
    .read = w25qxx_buffer_read,
    .write = w25qxx_buffer_write,
};
#endif

uint8_t hmac_key[] = "my_secret_key_12345";     // hmac密钥

// OTA工作状态
typedef enum{
    SVR_OTA_STATE_INIT = 0,
    SVR_OTA_STATE_START,
    SVR_OTA_STATE_DOWNLOAD,
    SVR_OTA_STATE_FINISH,
}svr_ota_state_t;

// OTA管理对象
static struct{
    svr_ota_state_t state;
    uint32_t download_num;
    uint16_t download_size;
} svr_ota_mgr = {
    .state = SVR_OTA_STATE_INIT,
    .download_num = 0,
    .download_size = 0,
};

// 公共区信息写入
void svr_ota_write_shared_info(shared_info_t* info){
    flash_dev.erase_sector(SHARED_ADDR, 1);
    flash_dev.write(SHARED_ADDR, (uint8_t*)info, 1*sizeof(shared_info_t));
}
// 公共区信息读取
void svr_ota_read_shared_info(shared_info_t* info){
    flash_dev.read(SHARED_ADDR, (uint8_t*)info, 1*sizeof(shared_info_t));
}
// 清除错误标志位
void svr_ota_clear_errorflag(void){
	shared_info_t info;
	svr_ota_read_shared_info(&info);
	info.error_flag = 0;
	svr_ota_write_shared_info(&info);
}

// 先别想太多，先解决当前的问题
void svr_ota_download_proc(void){
    static ota_protocol_info_t info;
	uint8_t signature_res[32];
	bsp_flash_status_t flash_res;

    if(svr_ota_mgr.state > SVR_OTA_STATE_INIT && 
	   svr_ota_mgr.state < SVR_OTA_STATE_FINISH &&
       ota_protocol_get_info(&info) != OTA_PROTOCOL_OK){
        return ;	// 在初始化结束后才开始处理接收数据
    }
    switch(svr_ota_mgr.state){
        case SVR_OTA_STATE_INIT:
            // 前置初始化工作
            ota_protocol_init();
            svr_ota_mgr.state = SVR_OTA_STATE_START;
            break;
        case SVR_OTA_STATE_START:
            // 等待/询问更新信号
            if(info.cmd == OTA_PROTOCOL_CMD_START){
                svr_ota_mgr.state = SVR_OTA_STATE_DOWNLOAD;
				svr_ota_mgr.download_num = 0;
                svr_ota_mgr.download_size = 0;

                flash_res = flash_dev.erase_sector(SECTION_BKU_ADDR, SECTION_NUM);
				if(flash_res != BSP_FLASH_OK){
					LOG_ERROR(TAG, "flash erase error");
				}
				hmac_start(hmac_key, sizeof(hmac_key)-1);       // 初始化HAMC状态机
				ota_protocol_send_ack();
			}
            break;
        case SVR_OTA_STATE_DOWNLOAD:
            if(info.cmd == OTA_PROTOCOL_CMD_WRITE){
                // 解密
                aes128_cbc_decrypt(info.payload, info.payload_len, info.pre_block, info.payload, info.pre_block);
				
                // 写入外存（前面4字节留给存储固件大小信息）
				LOG_DEBUG(TAG, "write to 0x%8x", SECTION_BKU_ADDR + SIZE_VALUE_BYTES + svr_ota_mgr.download_size);
                flash_res = flash_dev.write(SECTION_BKU_ADDR + SIZE_VALUE_BYTES + svr_ota_mgr.download_size, info.payload, info.payload_len);
				if(flash_res != BSP_FLASH_OK){
					LOG_ERROR(TAG, "flash write error");
				}
				
                // 更新签名
                if(svr_ota_mgr.download_size + info.payload_len > info.file_size){
                    // 去掉加密填充数据来进行解密
                    hmac_inner_proc(info.payload, info.file_size - svr_ota_mgr.download_size);
                    svr_ota_mgr.download_size = info.file_size;
                }
                else{
                    hmac_inner_proc(info.payload, info.payload_len);
                    svr_ota_mgr.download_size += info.payload_len;
                }
                svr_ota_mgr.download_num++;
            }
            else if(info.cmd == OTA_PROTOCOL_CMD_COMPLETE){
                svr_ota_mgr.state = SVR_OTA_STATE_FINISH;
            }
			else{
				LOG_ERROR(TAG, "cmd is %d", info.cmd);
				break;
			}
			ota_protocol_send_ack();    // 统一返回ACK
            break;
        case SVR_OTA_STATE_FINISH:
            // 签名校验
            hmac_outter_proc(signature_res);
            for(int i=0; i<32; i++){
                if(signature_res[i] != info.recv_signature[i]){
                    LOG_ERROR(TAG, "signature verification error %02x", signature_res[i]);
					svr_ota_mgr.state = SVR_OTA_STATE_START;
                    return ;
                }
            }
            // 写入固件大小信息在更新分区头部
            flash_res = flash_dev.write(SECTION_BKU_ADDR, (uint8_t*)&info.file_size, SIZE_VALUE_BYTES);
			if(flash_res != BSP_FLASH_OK){
				LOG_ERROR(TAG, "flash write error");
			}
            // 推送消息
            LOG_INFO(TAG, "download success");
			shared_info_t info;
			svr_ota_read_shared_info(&info);
			info.update_flag = 1;
			svr_ota_write_shared_info(&info);
			
            svr_ota_mgr.state = SVR_OTA_STATE_START;
            ota_protocol_send_ack();
			break;
    }
}

