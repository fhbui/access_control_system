#include "ili9341.h"
#include "bsp_spi.h"
#include <string.h>

ili9341_dev_t ili9341_dev;

/* ---------------- 底层私有宏：控制GPIO ---------------- */
#define LCD_CS_LOW()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
#define LCD_DC_CMD()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define LCD_DC_DATA()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)
#define LCD_RST_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

/**
 * @brief 写命令
 */
static void ili9341_write_cmd(uint8_t cmd) {
    LCD_CS_LOW();
    LCD_DC_CMD();
    bsp_spi_write(BSP_SPI_1, &cmd, 1);
    LCD_CS_HIGH();
}

/**
 * @brief 写8位数据
 */
static void ili9341_write_data8(uint8_t data) {
    LCD_CS_LOW();
    LCD_DC_DATA();
    bsp_spi_write(BSP_SPI_1, &data, 1);
    LCD_CS_HIGH();
}

/**
 * @brief 写16位颜色数据
 */
static void ili9341_write_data16(uint16_t data) {
    uint8_t buf[2];
    buf[0] = data >> 8;
    buf[1] = data & 0xFF;
    LCD_CS_LOW();
    LCD_DC_DATA();
    bsp_spi_write(BSP_SPI_1, buf, 2);
    LCD_CS_HIGH();
}

/**
 * @brief 硬件复位
 */
static void ili9341_reset(void) {
    LCD_RST_HIGH();
    HAL_Delay(50);
    LCD_RST_LOW();
    HAL_Delay(100);
    LCD_RST_HIGH();
    HAL_Delay(50);
}

/**
 * @brief 设置窗口区域
 */
void ili9341_set_window(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye) {
    ili9341_write_cmd(0x2A);
    ili9341_write_data8(xs >> 8);
    ili9341_write_data8(xs & 0xFF);
    ili9341_write_data8(xe >> 8);
    ili9341_write_data8(xe & 0xFF);

    ili9341_write_cmd(0x2B);
    ili9341_write_data8(ys >> 8);
    ili9341_write_data8(ys & 0xFF);
    ili9341_write_data8(ye >> 8);
    ili9341_write_data8(ye & 0xFF);

    ili9341_write_cmd(0x2C); // 准备写入显存
}

/**
 * @brief 初始化序列
 */
void ili9341_init(void) {
    ili9341_reset();

    ili9341_write_cmd(0xCF); ili9341_write_data8(0x00); ili9341_write_data8(0xC1); ili9341_write_data8(0x30);
    ili9341_write_cmd(0xED); ili9341_write_data8(0x64); ili9341_write_data8(0x03); ili9341_write_data8(0x12); ili9341_write_data8(0x81);
    ili9341_write_cmd(0xE8); ili9341_write_data8(0x85); ili9341_write_data8(0x00); ili9341_write_data8(0x78);
    ili9341_write_cmd(0xCB); ili9341_write_data8(0x39); ili9341_write_data8(0x2C); ili9341_write_data8(0x00); ili9341_write_data8(0x34); ili9341_write_data8(0x02);
    ili9341_write_cmd(0xF7); ili9341_write_data8(0x20);
    ili9341_write_cmd(0xEA); ili9341_write_data8(0x00); ili9341_write_data8(0x00);
    
    ili9341_write_cmd(0xC0); ili9341_write_data8(0x13); // Power control
    ili9341_write_cmd(0xC1); ili9341_write_data8(0x13); // Power control
    ili9341_write_cmd(0xC5); ili9341_write_data8(0x22); ili9341_write_data8(0x35); // VCM control
    ili9341_write_cmd(0xC7); ili9341_write_data8(0xBD); // VCM control2
    
    ili9341_write_cmd(0x36); ili9341_write_data8(0x08); // Memory Access Control (BGR order)
    ili9341_write_cmd(0x3A); ili9341_write_data8(0x55); // Pixel Format 16bit
    
    ili9341_write_cmd(0x11); // Exit Sleep
    HAL_Delay(120);
    ili9341_write_cmd(0x29); // Display ON

    ili9341_set_direction(0); // 默认竖屏
    ili9341_clear(ILI9341_WHITE);
}

/**
 * @brief 设置显示方向
 * @param dir: 0-竖屏, 1-横屏, 2-反向竖屏, 3-反向横屏
 */
void ili9341_set_direction(uint8_t dir) {
    ili9341_dev.dir = dir % 4;
    uint8_t reg_val = 0x08; // BGR 默认

    switch (ili9341_dev.dir) {
        case 0:
            ili9341_dev.width = ILI9341_LCD_WIDTH;
            ili9341_dev.height = ILI9341_LCD_HEIGHT;
            reg_val |= 0x00;
            break;
        case 1:
            ili9341_dev.width = ILI9341_LCD_HEIGHT;
            ili9341_dev.height = ILI9341_LCD_WIDTH;
            reg_val |= (1 << 5) | (1 << 6); // MV=1, MX=1
            break;
        // 其他方向可根据 0x36 寄存器自行扩展
    }
    ili9341_write_cmd(0x36);
    ili9341_write_data8(reg_val);
}

/**
 * @brief 全屏刷色
 */
void ili9341_clear(uint16_t color) {
    uint32_t total = ili9341_dev.width * ili9341_dev.height;
    ili9341_set_window(0, 0, ili9341_dev.width - 1, ili9341_dev.height - 1);
    
    for (uint32_t i = 0; i < total; i++) {
        ili9341_write_data16(color);
    }
}

/**
 * @brief 画点
 */
void ili9341_draw_point(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ili9341_dev.width || y >= ili9341_dev.height) return;
    ili9341_set_window(x, y, x, y);
    ili9341_write_data16(color);
}

/**
 * @brief 指定区域填充单一颜色（暂无DMA加速，加速需内部使用临时buffer）
 */
void ili9341_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color) {
    uint32_t total = (xe - xs + 1) * (ye - ys + 1);
    ili9341_set_window(xs, ys, xe, ye);
    for (uint32_t i = 0; i < total; i++) {
        ili9341_write_data16(color);
    }
}

/**
 * @brief  向指定区域推送像素数组 (用于显示图片、LVGL 刷屏)
 * @param  pdata: 像素颜色数组指针 (RGB565格式)
 */
void ili9341_draw_bitmap(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t *pdata){

}