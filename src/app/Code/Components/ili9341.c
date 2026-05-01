#include "ili9341.h"
#include "bsp_spi.h"
#include <string.h>
#include "spi.h"

ili9341_dev_t ili9341_dev;

/* ---------------- 底层私有宏：控制GPIO ---------------- */
#define LCD_CS_LOW()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
#define LCD_DC_CMD()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define LCD_DC_DATA()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)
#define LCD_RST_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define LCD_LED_ON()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
#define LCD_LED_OFF()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

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
    ili9341_write_cmd(0x21); // Display Inversion ON
    ili9341_write_cmd(0x36); ili9341_write_data8(0x08); // Memory Access Control 
    ili9341_write_cmd(0xB6); ili9341_write_data8(0x0A); ili9341_write_data8(0xA2); // Display Function Control
    ili9341_write_cmd(0x3A); ili9341_write_data8(0x55); // COLMOD: Pixel Format Set
    ili9341_write_cmd(0xF6); ili9341_write_data8(0x01); ili9341_write_data8(0x30); // Interface Control
    ili9341_write_cmd(0xB1); ili9341_write_data8(0x00); ili9341_write_data8(0x1B); // Frame Rate Control
    ili9341_write_cmd(0xF2); ili9341_write_data8(0x00); // Gamma Function Disable 
    ili9341_write_cmd(0x26); ili9341_write_data8(0x01); // Gamma curve selected 
    ili9341_write_cmd(0xE0); ili9341_write_data8(0x0F); ili9341_write_data8(0x35); ili9341_write_data8(0x31); ili9341_write_data8(0x0B); ili9341_write_data8(0x0E); ili9341_write_data8(0x06); ili9341_write_data8(0x49); ili9341_write_data8(0xA7); ili9341_write_data8(0x33); ili9341_write_data8(0x07); ili9341_write_data8(0x0F); ili9341_write_data8(0x03); ili9341_write_data8(0x0C); ili9341_write_data8(0x0A); ili9341_write_data8(0x00); // Set Gamma 
    ili9341_write_cmd(0xE1); ili9341_write_data8(0x00); ili9341_write_data8(0x0A); ili9341_write_data8(0x0F); ili9341_write_data8(0x04); ili9341_write_data8(0x11); ili9341_write_data8(0x08); ili9341_write_data8(0x36); ili9341_write_data8(0x58); ili9341_write_data8(0x4D); ili9341_write_data8(0x07); ili9341_write_data8(0x10); ili9341_write_data8(0x0C); ili9341_write_data8(0x32); ili9341_write_data8(0x34); ili9341_write_data8(0x0F); // Set Gamma
    
    ili9341_write_cmd(0x11); // Exit Sleep
    HAL_Delay(120);
    ili9341_write_cmd(0x29); // Display ON

    ili9341_set_direction(0); // 默认竖屏
    ili9341_clear(ILI9341_RED);
}

/**
 * @brief 设置显示方向
 * @param dir: 0-竖屏, 1-横屏, 2-反向竖屏, 3-反向横屏
 */
void ili9341_set_direction(uint8_t dir) {
    ili9341_dev.dir = dir % 4;
    uint8_t reg_val;

    switch (ili9341_dev.dir) {
        case 0:
            ili9341_dev.width = ILI9341_LCD_WIDTH;
            ili9341_dev.height = ILI9341_LCD_HEIGHT;
            reg_val = (1<<3)|(0<<6)|(0<<7);
            break;
        case 1:
            ili9341_dev.width = ILI9341_LCD_HEIGHT;
            ili9341_dev.height = ILI9341_LCD_WIDTH;
            reg_val = (1<<3)|(0<<7)|(1<<6)|(1<<5);
            break;
		case 2:
            ili9341_dev.width = ILI9341_LCD_WIDTH;
            ili9341_dev.height = ILI9341_LCD_HEIGHT;
            reg_val = (1<<3)|(1<<6)|(1<<7);
            break;
        case 3:
            ili9341_dev.width = ILI9341_LCD_HEIGHT;
            ili9341_dev.height = ILI9341_LCD_WIDTH;
            reg_val = (1<<3)|(1<<7)|(1<<5);
            break;
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
    uint32_t size = (xe - xs + 1) * (ye - ys + 1);
    ili9341_set_window(xs, ys, xe, ye);
    for (uint32_t i = 0; i < size; i++) {
        ili9341_write_data16(color);
    }
}

/**
 * @brief  向指定区域推送像素数组 (用于显示图片、LVGL 刷屏)
 * @param  color_p: 像素颜色数组指针 (RGB565格式)
 * @note   内不包含大小端切换，需在 lv_conf.h 中设置 #define LV_COLOR_16_SWAP 1
 */
void ili9341_draw_bitmap(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t *color_p){
    uint32_t total_bytes  = (xe - xs + 1) * (ye - ys + 1) * 2;
    ili9341_set_window(xs, ys, xe, ye);

    LCD_CS_LOW();
    LCD_DC_DATA();

    // 严谨起见，单次传输不要超过 65535。
    // LVGL 的缓冲区建议设置在 MY_DISP_HOR_RES * 10 左右，这通常只有几千字节。
    if (total_bytes <= 65535){
        bsp_spi_write_dma(BSP_SPI_1, (uint8_t *)color_p, (uint16_t)total_bytes);
    } 
    // 不要在此处调用设置窗口和拉高CS
//	while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
//	LCD_CS_HIGH();
}

ili9341_callback_t draw_bitmap_callback;
void* draw_bitmap_cb_usr_data;

/**
 * @brief  绑定推送像素数组结束后的回调函数
 * @note 没什么不好的，使用DMA进行传输就要接触底层外设，多定义一个绑定回调函数没啥不好的
 */
void ili9341_draw_bitmap_register_cb(ili9341_callback_t cb, void* usr_data){
    draw_bitmap_callback = cb;
    draw_bitmap_cb_usr_data = usr_data;
}

/**
 * @brief  推送像素数组结束后的相关处理，SPI传输完成调用
 */
//volatile uint8_t temp = 0;
void ili9341_draw_bitmap_finish(void){
	LCD_CS_HIGH();
	if (draw_bitmap_callback != NULL) {
//		temp += 1;
		draw_bitmap_callback(draw_bitmap_cb_usr_data);
	}
}