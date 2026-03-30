#include "as608.h"
#include "bsp_uart.h"
#include "bsp_dwt.h"
#include <string.h>

static uint8_t rx_buf[128];
static volatile uint8_t rx_flag = 0;

/**
 * @brief 统一发送与接收函数 (私有)
 * @param pkt_type 包类型
 * @param data 指令内容及参数
 * @param len 参数长度
 */
static uint8_t as608_send_and_receive(uint8_t cmd, uint8_t *params, uint16_t param_len) {
    uint8_t tx_buf[32];
    uint16_t sum = 0;
    uint16_t idx = 0;   // index

    // 1. 组包
    tx_buf[idx++] = 0xEF; tx_buf[idx++] = 0x01;             // Head
    tx_buf[idx++] = 0xFF; tx_buf[idx++] = 0xFF;             // Addr
    tx_buf[idx++] = 0xFF; tx_buf[idx++] = 0xFF;
    tx_buf[idx++] = 0x01;                                   // Type (Command)
    
    uint16_t pkg_len = param_len + 3;                       // Cmd(1) + Params(n) + Checksum(2)
    tx_buf[idx++] = pkg_len >> 8;
    tx_buf[idx++] = pkg_len & 0xFF;
    tx_buf[idx++] = cmd;
    
    for(uint16_t i=0; i<param_len; i++) tx_buf[idx++] = params[i];
    
    // 2. 计算校验和
    sum = 0x01 + (pkg_len >> 8) + (pkg_len & 0xFF) + cmd;
    for(uint16_t i=0; i<param_len; i++) sum += params[i];
    tx_buf[idx++] = sum >> 8;
    tx_buf[idx++] = sum & 0xFF;

    // 3. 发送并同步等待接收 (实际应用中建议通过信号量等待)
    rx_flag = 0;
    memset(rx_buf, 0, sizeof(rx_buf));
    bsp_uart_start_transmit(BSP_UART_1, tx_buf, idx);
    
    // 延时等待响应包 (根据实际修改，通常模块响应极快)
    uint32_t timeout = 500;
    while(!rx_flag && timeout--) HAL_Delay(1);
    
    if(!rx_flag) return AS608_ERR_COMM;
    return rx_buf[9]; // 返回响应包中的确认码字段
}


/**
 * @brief 采集图像
 */
uint8_t as608_cmd_get_image(void) {
    return as608_send_and_receive(0x01, NULL, 0);
}

/**
 * @brief 生成特征
 */ 
uint8_t as608_cmd_gen_char(uint8_t buffer_id) {
    uint8_t param = buffer_id;
    return as608_send_and_receive(0x02, &param, 1);
}

/**
 * @brief 搜索指纹
 */ 
uint8_t as608_cmd_search(uint8_t buffer_id, uint16_t start_page, uint16_t page_num, uint16_t *id, uint16_t *score) {
    uint8_t params[5];
    params[0] = buffer_id;
    params[1] = start_page >> 8; params[2] = start_page & 0xFF;
    params[3] = page_num >> 8;   params[4] = page_num & 0xFF;
    
    uint8_t res = as608_send_and_receive(0x1B, params, 5); // 高速搜索
    if(res == 0) {
        if(id) *id = (rx_buf[10] << 8) | rx_buf[11];
        if(score) *score = (rx_buf[12] << 8) | rx_buf[13];
    }
    return res;
}

/**
 * @brief 存储模板
 */ 
uint8_t as608_cmd_store_char(uint8_t buffer_id, uint16_t page_id) {
    uint8_t params[3];
    params[0] = buffer_id;
    params[1] = page_id >> 8; params[2] = page_id & 0xFF;
    return as608_send_and_receive(0x06, params, 3);
}

/**
 * @brief 合并特征
 */ 
uint8_t as608_cmd_reg_model(void) {
    return as608_send_and_receive(0x05, NULL, 0);
}