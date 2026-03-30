#include "main.h"
#include "usart.h"
#include "bsp_uart.h"

static const bsp_uart_bus_t uart_bus_table[BSP_UART_MAX] = {
    {(void*)&huart1, BSP_UART_MODE_ROLL, BSP_UART_MODE_DMA_IDLE, NULL},
	{(void*)&huart2, BSP_UART_MODE_ROLL, BSP_UART_MODE_DMA_IDLE, NULL},
};

/**
 * @brief 串口开启发送
 * @param id 串口ID
 * @param pdata 数据指针
 * @param len 数据长度
 */
bsp_uart_status_t bsp_uart_start_transmit(bsp_uart_id_t id, uint8_t* pdata, uint16_t len){
    const bsp_uart_bus_t* bus = &uart_bus_table[id];
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)bus->instance;
    HAL_StatusTypeDef status;

    if(bus->tx_mode == BSP_UART_MODE_ROLL){
        HAL_UART_Transmit(huart, pdata, len, 1000);
    }
    else if(bus->tx_mode == BSP_UART_MODE_IT){
        HAL_UART_Transmit_IT(huart, pdata, len);
    }
    else if(bus->tx_mode == BSP_UART_MODE_DMA){
        HAL_UART_Transmit_DMA(huart, pdata, len);
    }
    else if(bus->tx_mode == BSP_UART_MODE_DMA_IDLE){
        if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) == RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(huart); // 先清标志
            __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); // 后开中断
        }
        HAL_UART_Transmit_DMA(huart, pdata, len);
    }
}

/**
 * @brief 串口开启接收
 * @param id 串口ID
 * @param pdata 数据指针
 * @param len 数据长度
 */
bsp_uart_status_t bsp_uart_start_receive(bsp_uart_id_t id, uint8_t* pbuf, uint16_t len){
    const bsp_uart_bus_t* bus = &uart_bus_table[id];
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)bus->instance;
    HAL_StatusTypeDef status;
    
    if(bus->rx_mode == BSP_UART_MODE_ROLL){
        HAL_UART_Receive(huart, pbuf, len, 1000);
    }
    else if(bus->rx_mode == BSP_UART_MODE_IT){
        HAL_UART_Receive_IT(huart, pbuf, len);
    }
    else if(bus->rx_mode == BSP_UART_MODE_DMA){
        HAL_UART_Receive_DMA(huart, pbuf, len);
    }
    else if(bus->rx_mode == BSP_UART_MODE_DMA_IDLE){
        if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) == RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(huart); // 先清标志
            __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); // 后开中断
        }
        HAL_UART_Receive_DMA(huart, pbuf, len);
    }
}

int fputc(int ch, FILE *f){
    // 发送单个字符
    bsp_uart_start_transmit(BSP_UART_2, (uint8_t *)&ch, 1);
    return ch;
}