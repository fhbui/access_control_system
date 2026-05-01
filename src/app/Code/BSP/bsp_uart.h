#ifndef __BSP_UART_H
#define __BSP_UART_H

#include <stdint.h>

// 串口ID号
typedef enum{
	BSP_UART_1= 0,
	BSP_UART_2,
    BSP_UART_3,
    BSP_UART_MAX,
}bsp_uart_id_t;

// 串口使用模式
typedef enum {
    BSP_UART_MODE_ROLL = 0,
    BSP_UART_MODE_IT, 
    BSP_UART_MODE_DMA,
    BSP_UART_MODE_DMA_IDLE,
} bsp_uart_mode_t;

typedef void (*bsp_uart_callback_t)(uint8_t*);

typedef enum{
    BSP_UART_OK = 0,
    BSP_UART_ERROR
}bsp_uart_status_t;

bsp_uart_status_t bsp_uart_start_transmit(bsp_uart_id_t id, uint8_t* pdata, uint16_t len);
bsp_uart_status_t bsp_uart_start_receive(bsp_uart_id_t id, uint8_t* pbuf, uint16_t len);
void bsp_uart_register_cb(bsp_uart_id_t id, bsp_uart_callback_t cb);
void bsp_uart_exec_it(bsp_uart_id_t id);

void bus_uart_set_recv_info(bsp_uart_id_t id, uint16_t recv_len);
void bsp_uart_get_recv_info(bsp_uart_id_t id, uint8_t** rx_buf, uint16_t* rx_size, uint16_t* recv_len);

#endif