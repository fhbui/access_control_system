#include "main.h"
#include "usart.h"
#include "bsp_uart.h"

typedef struct{
    void* instance;
    bsp_uart_mode_t tx_mode;
    bsp_uart_mode_t rx_mode;
    uint8_t* rx_buf;
    uint16_t rx_size;
    uint16_t recv_len;
    uint8_t* cb_param;
    bsp_uart_callback_t rx_callback;
}bsp_uart_bus_t;

static bsp_uart_bus_t uart_bus_table[BSP_UART_MAX] = {
    {(void*)&huart1, BSP_UART_MODE_ROLL, BSP_UART_MODE_DMA_IDLE, NULL, 0, 0, NULL, NULL},
	{(void*)&huart2, BSP_UART_MODE_ROLL, BSP_UART_MODE_DMA_IDLE, NULL, 0, 0, NULL, NULL},
    {(void*)&huart3, BSP_UART_MODE_ROLL, BSP_UART_MODE_DMA_IDLE, NULL, 0, 0, NULL, NULL},
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
        if (__HAL_UART_GET_FLAG(huart, UART_IT_IDLE) != RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(huart); // 先清标志
        }
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
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
    bsp_uart_bus_t* bus = &uart_bus_table[id];
    UART_HandleTypeDef* huart = (UART_HandleTypeDef*)bus->instance;
    HAL_StatusTypeDef status;

    bus->rx_buf = pbuf;
    bus->rx_size = len;
    
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
        if (__HAL_UART_GET_FLAG(huart, UART_IT_IDLE) != RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(huart); // 先清标志
        }
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); // 后开中断
        HAL_UART_Receive_DMA(huart, pbuf, len);
    }
}

void bsp_uart_register_cb(bsp_uart_id_t id, bsp_uart_callback_t cb){
    bsp_uart_bus_t* bus = &uart_bus_table[id];
    bus->rx_callback = cb;
}

// 设置接收信息（主要是设置接收长度）
void bus_uart_set_recv_info(bsp_uart_id_t id, uint16_t recv_len){
    bsp_uart_bus_t* bus = &uart_bus_table[id];
    bus->recv_len = recv_len;
}

// 获取接收的相关信息
void bsp_uart_get_recv_info(bsp_uart_id_t id, uint8_t** rx_buf, uint16_t* rx_size, uint16_t* recv_len){
    bsp_uart_bus_t* bus = &uart_bus_table[id];
    if(rx_buf != NULL){
        *rx_buf = bus->rx_buf;
    }
    if(rx_size != NULL){
        *rx_size = bus->rx_size;
    }
    if(recv_len != NULL){
        *recv_len = bus->recv_len;
    }
}

// 串口中断中调用
void bsp_uart_exec_it(bsp_uart_id_t id){
    bsp_uart_bus_t* bus = &uart_bus_table[id];
    if(bus->rx_callback != NULL){
        bus->rx_callback(bus->cb_param);
    }
}

int fputc(int ch, FILE *f){
    // 发送单个字符
    bsp_uart_start_transmit(BSP_UART_1, (uint8_t *)&ch, 1);
    return ch;
}