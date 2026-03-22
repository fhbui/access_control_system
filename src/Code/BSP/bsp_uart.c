#include "main.h"
#include "bsp_uart.h"

static const bsp_uart_bus_t uart_bus_table[BSP_UART_MAX] = {
    NULL
};

bsp_uart_status_t bsp_uart_start_transmit(bsp_uart_id_t id, uint8_t* pdata, uint16_t len){
    const bsp_uart_bus_t* bus = &uart_bus_table[id];
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)bus->instance;
    HAL_StatusTypeDef status;

    if(bus->tx_mode == UART_MODE_ROLL){

    }
    else if(bus->tx_mode == UART_MODE_IT){

    }
}

bsp_uart_status_t bsp_uart_start_receive(bsp_uart_id_t id){
    const bsp_uart_bus_t* bus = &uart_bus_table[id];
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)bus->instance;
    HAL_StatusTypeDef status;
    
    if(bus->rx_mode == UART_MODE_ROLL){

    }
    else if(bus->rx_mode == UART_MODE_IT){

    }
    else if(bus->rx_mode == UART_MODE_DMA){

    }
    else if(bus->rx_mode == UART_MODE_DMA_IDLE){
        if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) == RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(huart); // 先清标志
            __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); // 后开中断
        }
        HAL_UART_Receive_DMA(huart, bus->rx_buf, bus->rx_size);
    }
}