#ifndef __ILI9341_H__
#define __ILI9341_H__

#include "main.h"
#include <stdint.h>

/* 屏幕尺寸定义 */
#define ILI9341_LCD_WIDTH   240
#define ILI9341_LCD_HEIGHT  320

/* 常用颜色 */
#define ILI9341_WHITE       0xFFFF
#define ILI9341_BLACK       0x0000
#define ILI9341_RED         0xF800
#define ILI9341_GREEN       0x07E0
#define ILI9341_BLUE        0x001F

/* 设备控制结构体 */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  dir;        // 方向: 0-竖屏, 1-横屏...
    uint16_t point_color;
    uint16_t back_color;
}ili9341_dev_t;

typedef void (*ili9341_callback_t)(void* usr_data);
extern ili9341_dev_t ili9341_dev;

/* ---------------- 基础接口 ---------------- */
void ili9341_init(void);
void ili9341_set_direction(uint8_t dir);
void ili9341_set_window(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void ili9341_clear(uint16_t color);

/* ---------------- 绘图接口 ---------------- */
void ili9341_draw_point(uint16_t x, uint16_t y, uint16_t color);
void ili9341_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);
void ili9341_draw_bitmap(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t *color_p);
void ili9341_draw_bitmap_register_cb(ili9341_callback_t cb, void* usr_data);
void ili9341_draw_bitmap_finish(void);

#endif