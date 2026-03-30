#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "main.h"

// I2C 类型枚举，既能使用软件又能同时使用硬件
typedef enum {
    I2C_TYPE_HW = 0,
    I2C_TYPE_SW
}i2c_type_t;

// ID 索引
typedef enum {
    BSP_I2C_1 = 0,
    BSP_I2C_SOFT_1,
    BSP_I2C_MAX
}bsp_i2c_id_t;

typedef enum{
    SDA_IO_STA_OUT = 0,
    SDA_IO_STA_IN,
}sda_io_status_t;

// 软件 I2C 引脚配置结构体
typedef struct {
    GPIO_TypeDef* scl_port;
    uint16_t      scl_pin;
    GPIO_TypeDef* sda_port;
    uint16_t      sda_pin;
    // uint8_t is_external_pullup;
    // sda_io_status_t sda_io_sta;
}soft_i2c_cfg_t;

// I2C 实例结构体
typedef struct {
    i2c_type_t type;
    void*      instance;   // 硬件为 I2C_HandleTypeDef*，软件为 soft_i2c_cfg_t*
}bsp_i2c_bus_t;

// 状态返回值
typedef enum {
    BSP_I2C_OK = 0,
    BSP_I2C_ERROR
}bsp_i2c_status_t;

// 统一API
bsp_i2c_status_t bsp_i2c_write(bsp_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size, uint8_t *pdata, uint16_t len);
bsp_i2c_status_t bsp_i2c_read(bsp_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size, uint8_t *pdata, uint16_t len);

#endif