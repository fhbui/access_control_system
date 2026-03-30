#include "ft6336.h"
#include "bsp_i2c.h"
#include "bsp_dwt.h"
#include "log.h"

static const char* TAG = "ft6336";

/* 底层控制：复位引脚映射 */
#define FT6336_RST_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define FT6336_RST_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)

ft6336_data_t fp_data;

/**
 * @brief 写入寄存器
 */
void ft6336_write_reg(uint8_t reg, uint8_t *buf, uint16_t len){
    bsp_i2c_write(BSP_I2C_SOFT_1, FT6336_ADDR, reg, 1, buf, len);
}

/**
 * @brief 读取寄存器
 */
void ft6336_read_reg(uint8_t reg, uint8_t *buf, uint16_t len){
    bsp_i2c_read(BSP_I2C_SOFT_1, FT6336_ADDR, reg, 1, buf, len);
}

/**
 * @brief 初始化芯片
 */
uint8_t ft6336_init(void){
    uint8_t id = 0;

    // 硬件复位
    FT6336_RST_LOW();
    bsp_Delayms(20);
    FT6336_RST_HIGH();
    bsp_Delayms(200);

    // 读取 Vendor ID 校验
    ft6336_read_reg(FT_REG_ID_G_FOCALTECH, &id, 1);
    if (id != 0x11) return 1;

    // (可选) 设置模式为普通检测模式
    uint8_t mode = 0x00;
    ft6336_write_reg(FT_REG_MODE, &mode, 1);

    return 0;
}

/**
 * @brief 获取原始坐标数据
 * @param data: 存储读取结果的结构体指针
 * @return 0: 成功, 1: 失败或无触摸
 */
uint8_t ft6336_read_points(ft6336_data_t *data){
    uint8_t status = 0;
    uint8_t buf[4];

    // 读取当前触摸点数
    ft6336_read_reg(FT_REG_NUM_FINGER, &status, 1);
    data->point_num = status & 0x0F;

    if (data->point_num == 0 || data->point_num > FT6336_MAX_TOUCH) {
        return 1;
    }

    // 分次读取坐标点 (也可以一次性读取多个字节优化，但这里为了逻辑清晰分开读)
    for (uint8_t i = 0; i < data->point_num; i++) {
        uint8_t reg_addr = (i == 0) ? FT_REG_TP1_BASE : FT_REG_TP2_BASE;
        ft6336_read_reg(reg_addr, buf, 4);

        data->points[i].x = ((uint16_t)(buf[0] & 0x0F) << 8) | buf[1];
        data->points[i].y = ((uint16_t)(buf[2] & 0x0F) << 8) | buf[3];
        data->points[i].event = (buf[0] & 0xC0) >> 6;
        data->points[i].id = (buf[2] & 0xF0) >> 4;
    }
    return 0;
}

void ft6336_scan(void){
    ft6336_read_points(&fp_data);
}

void ft6336_test(void){
    uint8_t res = 0x00;

    // 硬件复位（非常重要）
    FT6336_RST_LOW();
    bsp_Delayms(20);
    FT6336_RST_HIGH();
    bsp_Delayms(200);

    // 读取 Vendor ID 校验
    ft6336_read_reg(FT_REG_ID_G_FOCALTECH, &res, 1);
	LOG_DEBUG(TAG, "res is 0x%02x", res);
}