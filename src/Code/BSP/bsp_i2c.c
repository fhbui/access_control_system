#include "bsp_i2c.h"
#include "log.h"

// 软件I2C底层实现

// 软件 I2C 延迟
static void soft_i2c_delay(void){
    for(volatile int i=0; i<50; i++); 
}

static void soft_i2c_start(soft_i2c_cfg_t* cfg){
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 1);
    soft_i2c_delay();

    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 0);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
}

static void soft_i2c_send_byte(soft_i2c_cfg_t* cfg, uint8_t byte){

    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    for(int i=0; i<8; i++){
        // MSB先行
        HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, (byte & 0x80)?1:0);
        byte <<= 1;
        soft_i2c_delay();
        HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
        soft_i2c_delay();
        HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    }
}

static uint8_t soft_i2c_read_byte(soft_i2c_cfg_t* cfg){
    // 释放 SDA 线，让从机可以驱动它
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 1); 
    soft_i2c_delay();
    
    uint8_t recv_byte = 0x00, temp;
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    soft_i2c_delay();

    for(int i=0; i<8; i++){
        // MSB先行
        HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
        soft_i2c_delay();
        temp = HAL_GPIO_ReadPin(cfg->sda_port, cfg->sda_pin);
        recv_byte <<= 1;
        if(temp)    recv_byte++;
        HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
        soft_i2c_delay();
    }
    return recv_byte;
}

static void soft_i2c_send_ack(soft_i2c_cfg_t* cfg){
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 0);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    soft_i2c_delay();
}

static void soft_i2c_send_nack(soft_i2c_cfg_t* cfg){
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 1);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    soft_i2c_delay();
}

static uint8_t soft_i2c_wait_ack(soft_i2c_cfg_t* cfg){
    // 如果配置为推挽输出，则需要切换GPIO工作模式为输入模式，以便从机驱动电平
    
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 1);  // 显式释放 SDA（建议）
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
    soft_i2c_delay();
    uint8_t ack = HAL_GPIO_ReadPin(cfg->sda_port, cfg->sda_pin);
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    soft_i2c_delay();
    return ack;
}

static void soft_i2c_stop(soft_i2c_cfg_t* cfg){
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 0);
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 0);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->scl_port, cfg->scl_pin, 1);
    soft_i2c_delay();
    HAL_GPIO_WritePin(cfg->sda_port, cfg->sda_pin, 1);
}

/**
 * @brief 软件 I2C 发送设备地址及内存起始地址
 * @param cfg 软件 I2C 配置结构体指针
 * @param dev_addr 8 位设备地址 (需提前左移)
 * @param mem_addr 内存/寄存器起始地址
 * @param mem_size 内存地址长度 (1: 8bits, 2: 16bits)
 * @return bsp_i2c_status_t 成功返回 BSP_I2C_OK
 */
static bsp_i2c_status_t soft_i2c_send_mem_addr(soft_i2c_cfg_t* cfg, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size) {
    // 1. 发送设备写地址
    soft_i2c_start(cfg);
    soft_i2c_send_byte(cfg, dev_addr & 0xFE); // 强制最低位为0，表示写
    if (soft_i2c_wait_ack(cfg) != 0) return BSP_I2C_ERROR;

    // 2. 发送内存地址
    if (mem_size == 2) { // 16位地址 (MSB先行)
        soft_i2c_send_byte(cfg, (uint8_t)(mem_addr >> 8));
        if (soft_i2c_wait_ack(cfg) != 0) return BSP_I2C_ERROR;
    }
    soft_i2c_send_byte(cfg, (uint8_t)(mem_addr & 0xFF));
    if (soft_i2c_wait_ack(cfg) != 0) return BSP_I2C_ERROR;

    return BSP_I2C_OK;
}


static soft_i2c_cfg_t soft_i2c_cfg1 = {GPIOB, GPIO_PIN_8, GPIOB, GPIO_PIN_9};

// 静态配置表（实现通过ID找到相应的实例）
static const bsp_i2c_bus_t i2c_bus_table[BSP_I2C_MAX] = {
    {I2C_TYPE_HW, NULL},      // BSP_I2C_1
    {I2C_TYPE_SW, &soft_i2c_cfg1}    // BSP_I2C_SOFT_1
};


/**
 * @brief I2C 统一写入接口 (支持硬件屏蔽与软件模拟)
 * @param id I2C 总线枚举 ID
 * @param dev_addr 8 位从机设备地址
 * @param mem_addr 目标寄存器/内存地址
 * @param mem_size 寄存器地址字节数 (1 或 2)
 * @param pdata 待写入数据缓冲区指针
 * @param len 待写入数据长度 (字节)
 * @return bsp_i2c_status_t 传输状态
 */
bsp_i2c_status_t bsp_i2c_write(bsp_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size, uint8_t *pdata, uint16_t len){
    const bsp_i2c_bus_t* bus = &i2c_bus_table[id];

    if(bus->type == I2C_TYPE_HW){

    }
    else if(bus->type == I2C_TYPE_SW){
        soft_i2c_cfg_t* cfg = (soft_i2c_cfg_t*)bus->instance;
        // 1. 发送设备地址和寄存器地址
        if (soft_i2c_send_mem_addr(cfg, dev_addr, mem_addr, mem_size) != BSP_I2C_OK) {
            soft_i2c_stop(cfg);
            return BSP_I2C_ERROR;
        }
        // 2. 循环发送数据
        for (uint16_t i = 0; i < len; i++) {
            soft_i2c_send_byte(cfg, pdata[i]);
            if (soft_i2c_wait_ack(cfg) != 0) {
                soft_i2c_stop(cfg);
                return BSP_I2C_ERROR;
            }
        }
        // 3. 停止信号
        soft_i2c_stop(cfg);
        return BSP_I2C_OK;
    }
}

/**
 * @brief I2C 统一读取接口 (支持硬件屏蔽与软件模拟)
 * @param id I2C 总线枚举 ID
 * @param dev_addr 8 位从机设备地址
 * @param mem_addr 目标寄存器/内存地址
 * @param mem_size 寄存器地址字节数 (1 或 2)
 * @param pdata 数据接收缓冲区指针
 * @param len 期望读取的数据长度 (字节)
 * @return bsp_i2c_status_t 传输状态
 */
bsp_i2c_status_t bsp_i2c_read(bsp_i2c_id_t id, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_size, uint8_t *pdata, uint16_t len){
    const bsp_i2c_bus_t* bus = &i2c_bus_table[id];

    if(bus->type == I2C_TYPE_HW){
		
    }
    else if(bus->type == I2C_TYPE_SW){
        soft_i2c_cfg_t* cfg = (soft_i2c_cfg_t*)bus->instance;
        // 1. 先写入要读取的寄存器地址
        if (soft_i2c_send_mem_addr(cfg, dev_addr, mem_addr, mem_size) != BSP_I2C_OK) {
            soft_i2c_stop(cfg);
            return BSP_I2C_ERROR;
        }
        // 2. 发送重复起始信号 (Restart)，切换到读取模式
        soft_i2c_start(cfg); 
        soft_i2c_send_byte(cfg, dev_addr | 0x01); // 最低位置1，表示读
        if (soft_i2c_wait_ack(cfg) != 0) {
            soft_i2c_stop(cfg);
            return BSP_I2C_ERROR;
        }
        // 3. 循环读取数据
        for (uint16_t i = 0; i < len; i++) {
            pdata[i] = soft_i2c_read_byte(cfg);
            // 如果是最后一个字节，发送 NACK，否则发送 ACK
            if (i == (len - 1)) {
                soft_i2c_send_nack(cfg);
            } else {
                soft_i2c_send_ack(cfg);
            }
        }
        // 4. 停止信号
        soft_i2c_stop(cfg);
        return BSP_I2C_OK;
    }
}
