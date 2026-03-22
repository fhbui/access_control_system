#ifndef __BSP_UART_H
#define __BSP_UART_H

#include <stdint.h>

typedef enum{
	BSP_UART_1= 0,
    BSP_UART_MAX,
}bsp_uart_id_t;

typedef enum {
    UART_MODE_ROLL = 0,
    UART_MODE_IT, 
    UART_MODE_DMA,
    UART_MODE_DMA_IDLE,
} bsp_uart_mode_t;

typedef struct{
    void* instance;
    bsp_uart_mode_t tx_mode;
    bsp_uart_mode_t rx_mode;
    uint8_t* rx_buf;
    uint16_t rx_size;
    void (*app_rx_callback)(uint8_t *pdata, uint16_t len);
}bsp_uart_bus_t;

typedef enum{
    BSP_UART_OK = 0,
    BSP_UART_ERROR
}bsp_uart_status_t;

#endif