#include "bsp_spi.h"

static const bsp_spi_bus_t spi_bus_table[BSP_SPI_MAX] = {
    NULL
};

/**
 * @brief  SPI 通用交换函数 (支持 轮询/中断/DMA)
 * @param  id      SPI 总线标识
 * @param  p_tx    发送缓冲区 (NULL 表示不发送)
 * @param  p_rx    接收缓冲区 (NULL 表示不接收)
 * @param  len     数据长度
 * @param  mode    传输模式 (POLLING / IT / DMA)
 * @param  timeout 超时时间 (仅在 POLLING 模式下有效)
 * @return bsp_spi_status_t 
 */
bsp_spi_status_t bsp_spi_exchange(bsp_spi_id_t id, const uint8_t *p_tx, uint8_t *p_rx, 
                                 uint16_t len, bsp_spi_mode_t mode, uint32_t timeout) 
{
    SPI_HandleTypeDef *hspi = spi_bus_table[id].instance;
    HAL_StatusTypeDef status = HAL_ERROR;

    switch (mode) {
        case BSP_SPI_MODE_POLL:
            if (p_tx && p_rx) status = HAL_SPI_TransmitReceive(hspi, (uint8_t*)p_tx, p_rx, len, timeout);
            else if (p_tx)    status = HAL_SPI_Transmit(hspi, (uint8_t*)p_tx, len, timeout);
            else              status = HAL_SPI_Receive(hspi, p_rx, len, timeout);
            break;

        case BSP_SPI_MODE_IT:
            while (HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY);
            if (p_tx && p_rx) status = HAL_SPI_TransmitReceive_IT(hspi, (uint8_t*)p_tx, p_rx, len);
            else if (p_tx)    status = HAL_SPI_Transmit_IT(hspi, (uint8_t*)p_tx, len);
            else              status = HAL_SPI_Receive_IT(hspi, p_rx, len);
            break;

        case BSP_SPI_MODE_DMA:
            while (HAL_SPI_GetState(hspi) != HAL_SPI_STATE_READY);
            if (p_tx && p_rx) status = HAL_SPI_TransmitReceive_DMA(hspi, (uint8_t*)p_tx, p_rx, len);
            else if (p_tx)    status = HAL_SPI_Transmit_DMA(hspi, (uint8_t*)p_tx, len);
            else              status = HAL_SPI_Receive_DMA(hspi, p_rx, len);
            break;
            
        default:
            return BSP_SPI_ERROR;
    }

    return (status == HAL_OK) ? BSP_SPI_OK : BSP_SPI_ERROR;
}

/**
 * @brief  SPI 仅发送数据 (内联包装函数)
 * @param  id    SPI 总线标识枚举
 * @param  pdata 待发送数据的起始地址指针
 * @param  len   发送数据的长度 (字节) 
 * @return bsp_spi_status_t 传输结果状态 
 * @note 通过 inline 关键字消除函数调用开销，提高传输效率。
 */
inline bsp_spi_status_t bsp_spi_write(bsp_spi_id_t id, uint8_t *data, uint16_t len) {
    return bsp_spi_exchange(id, data, NULL, len, BSP_SPI_MODE_POLL, 100);
}

inline bsp_spi_status_t bsp_spi_write_it(bsp_spi_id_t id, uint8_t *data, uint16_t len) {
    return bsp_spi_exchange(id, data, NULL, len, BSP_SPI_MODE_IT, 100);
}

inline bsp_spi_status_t bsp_spi_write_dma(bsp_spi_id_t id, uint8_t *data, uint16_t len) {
    return bsp_spi_exchange(id, data, NULL, len, BSP_SPI_MODE_DMA, 100);
}

/**
 * @brief  SPI 仅读取数据 (内联包装函数)
 * @param  id    SPI 总线标识枚举
 * @param  pbuf  接收存储区的起始地址指针
 * @param  len   期望读取的字节数
 * @return bsp_spi_status_t 传输结果状态
 */
inline bsp_spi_status_t bsp_spi_read(bsp_spi_id_t id, uint8_t *buf, uint16_t len) {
    return bsp_spi_exchange(id, NULL, buf, len, BSP_SPI_MODE_POLL, 100);
}

inline bsp_spi_status_t bsp_spi_read_it(bsp_spi_id_t id, uint8_t *buf, uint16_t len) {
    return bsp_spi_exchange(id, NULL, buf, len, BSP_SPI_MODE_IT, 100);
}

inline bsp_spi_status_t bsp_spi_read_dma(bsp_spi_id_t id, uint8_t *buf, uint16_t len) {
    return bsp_spi_exchange(id, NULL, buf, len, BSP_SPI_MODE_DMA, 100);
}
