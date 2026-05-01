#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include <stdint.h>

typedef enum{
    BSP_SPI_1 = 0,
	BSP_SPI_2,
    BSP_SPI_3,
    BSP_SPI_MAX
}bsp_spi_id_t;

typedef enum{
    BSP_SPI_MODE_POLL = 0,
    BSP_SPI_MODE_IT,
    BSP_SPI_MODE_DMA
}bsp_spi_mode_t;

typedef struct{
    void* instance;
    // bsp_spi_mode_t transmit_mode;    // 不应该配置死
    // GPIO_TypeDef* cs_port;
    // uint16_t cs_pin;
    // uint8_t* recv_buf;
    // uint16_t recv_size;
}bsp_spi_bus_t;

typedef enum{
    BSP_SPI_OK = 0,
    BSP_SPI_ERROR
}bsp_spi_status_t;

bsp_spi_status_t bsp_spi_exchange(bsp_spi_id_t id, const uint8_t *p_tx, uint8_t *p_rx, 
                                 uint16_t len, bsp_spi_mode_t mode, uint32_t timeout);
inline bsp_spi_status_t bsp_spi_write(bsp_spi_id_t id, uint8_t *data, uint16_t len);
inline bsp_spi_status_t bsp_spi_write_it(bsp_spi_id_t id, uint8_t *data, uint16_t len);
inline bsp_spi_status_t bsp_spi_write_dma(bsp_spi_id_t id, uint8_t *data, uint16_t len);
inline bsp_spi_status_t bsp_spi_read(bsp_spi_id_t id, uint8_t *buf, uint16_t len);
inline bsp_spi_status_t bsp_spi_read_it(bsp_spi_id_t id, uint8_t *buf, uint16_t len);
inline bsp_spi_status_t bsp_spi_read_dma(bsp_spi_id_t id, uint8_t *buf, uint16_t len);

#endif