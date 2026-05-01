// 只负责解包程序协议
#include "ota_protocol.h"
#include "bsp_uart.h"
#include <stdio.h>
#include <string.h>
#include "log.h"

static const char* TAG = "ota_protocol";

#define RECV_MAX_LEN    134
static uint8_t ota_recv_buf[RECV_MAX_LEN];

// 串口操作对象
static struct{
    volatile uint8_t recv_flag;
    uint8_t* recv_buf;
    uint16_t buf_len;
}recv_dev = {
    .recv_flag = 0,
    .recv_buf = ota_recv_buf,
    .buf_len = RECV_MAX_LEN,
};

// 交给串口底层调用
static void recv_callback(uint8_t* usr_data){
    recv_dev.recv_flag = 1;
}

/**
 * @brief 初始化 OTA 协议层所需底层设备
 */
void ota_protocol_init(void){
    bsp_uart_register_cb(BSP_UART_3, recv_callback);
    bsp_uart_start_receive(BSP_UART_3, recv_dev.recv_buf, recv_dev.buf_len);
}

/**
 * @brief 发送 ACK (确认) 信号
 */
void ota_protocol_send_ack(void){
    uint8_t ack = 0x06;
    bsp_uart_start_transmit(BSP_UART_3, &ack, 1);
}

/**
 * @brief 发送 NACK (非确认) 信号
 */
void ota_protocol_send_nack(void){
    uint8_t nack = 0x07;
    bsp_uart_start_transmit(BSP_UART_3, &nack, 1);
}

/**
 * @brief 计算 CRC16 校验值
 * @details 采用 CRC16-MODBUS 算法（多项式 0xA001）计算数据的校验码。
 * @param data 需要校验的数据缓冲区指针
 * @param len  需要校验的数据长度
 * @return uint16_t 计算得到的 16 位 CRC 校验值
 */
static uint16_t cal_crc16(uint8_t* data, uint8_t len){
    uint16_t crc = 0xFFFF;
    for(int i=0; i<len; i++){
        crc ^= data[i];
        for(int j=0; j<8; j++){
            if((crc&1) != 0)    crc = (crc>>1)^0xA001;
            else                crc >>= 1;
        }
    }
    return crc;
}

/**
 * @brief 解析并获取 OTA 协议包信息
 * @param info 指向协议信息结构体的指针，用于存储解析结果
 * @return ota_protocol_status_t 解析状态：
 * @note 内部不会自动返回ACK，需要外部进行返回
 */
ota_protocol_status_t ota_protocol_get_info(ota_protocol_info_t* info){
    if(recv_dev.recv_flag == 0){
        return OTA_PROTOCOL_IDLE;
    }
	recv_dev.recv_flag = 0;		// 清除标志位
	
    if(recv_dev.recv_buf[0] != 0x55 || recv_dev.recv_buf[1] != 0xAA){
        return OTA_PROTOCOL_ERROR;
    }
	
    uint8_t payload_len = recv_dev.recv_buf[2];
    uint16_t crc_res = cal_crc16(&recv_dev.recv_buf[2], payload_len + 2);   // 包括了len和cmd
    int crc_index = 4 + payload_len;
    if(crc_res != (uint16_t)(recv_dev.recv_buf[crc_index] | (recv_dev.recv_buf[crc_index+1]<<8))){
        ota_protocol_send_nack();   // 发送NACK
        return OTA_PROTOCOL_ERROR;
    }
    
    info->payload_len = payload_len;
    info->cmd = recv_dev.recv_buf[3];
    info->payload = &recv_dev.recv_buf[4];

    switch(info->cmd){
        case OTA_PROTOCOL_CMD_IV:
            LOG_DEBUG(TAG, "iv");
            memcpy(info->pre_block, info->payload, 16);
            break;
        case OTA_PROTOCOL_CMD_SIGN:
            LOG_DEBUG(TAG, "sign");
            memcpy(info->recv_signature, info->payload, 32);
            break;
        case OTA_PROTOCOL_CMD_SIZE:
            info->file_size = (uint32_t)(info->payload[0] | info->payload[1]<<8);
            LOG_DEBUG(TAG, "file size is %d", info->file_size);
            break;
        default:
            return OTA_PROTOCOL_OK;
    }
    ota_protocol_send_ack();
    return OTA_PROTOCOL_IDLE;
}


