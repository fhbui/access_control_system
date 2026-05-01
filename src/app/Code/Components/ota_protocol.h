#ifndef __OTA_PROTOCOL_H
#define __OTA_PROTOCOL_H

#include "main.h"

typedef enum{
    OTA_PROTOCOL_CMD_START = 0x00,
    OTA_PROTOCOL_CMD_COMPLETE,
    OTA_PROTOCOL_CMD_WRITE,
    OTA_PROTOCOL_CMD_COPY,
	OTA_PROTOCOL_CMD_IV,
	OTA_PROTOCOL_CMD_SIGN,
    OTA_PROTOCOL_CMD_SIZE,
}ota_protocol_cmd_t;

typedef struct{
    ota_protocol_cmd_t cmd;
    uint8_t* payload;
    uint16_t payload_len;
    uint8_t pre_block[16];       // 上一块密文，用于CBC
    uint8_t recv_signature[32];  // 接收到的签名值
    uint32_t file_size;     // 更新固件大小（用于辅助签名校验）
}ota_protocol_info_t;

typedef enum{
    OTA_PROTOCOL_OK = 0,
    OTA_PROTOCOL_ERROR,
    OTA_PROTOCOL_IDLE       // 没事情或不需要外部处理
}ota_protocol_status_t;

void ota_protocol_init(void);
void ota_protocol_send_ack(void);
void ota_protocol_send_nack(void);
ota_protocol_status_t ota_protocol_get_info(ota_protocol_info_t* info);

#endif
