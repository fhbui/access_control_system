#ifndef __FT6336_H__
#define __FT6336_H__

#include <stdint.h>

/* ---------------- 配置定义 ---------------- */
#define FT6336_MAX_TOUCH        2       // 支持的最大触摸点数
#define FT6336_ADDR             0x70    // 器件地址 (7位地址: 0x38, 左移后为 0x70)

/* ---------------- 寄存器定义 ---------------- */
#define FT_REG_MODE             0x00    // 工作模式
#define FT_REG_NUM_FINGER       0x02    // 触摸点数
#define FT_REG_TP1_BASE         0x03    // 第一个触控点寄存器基址
#define FT_REG_TP2_BASE         0x09    // 第二个触控点寄存器基址
#define FT_REG_ID_G_FOCALTECH   0xA8    // Vendor ID (默认0x11)
#define FT_REG_LIB_VERSION      0xA1    // 版本号

/* ---------------- 数据结构 ---------------- */
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  event; // 0: Down (按下), 1: Up (抬起), 2: Contact (接触), 3: No event
    uint8_t  id;    // 触控点 ID
} ft6336_point_t;

/**
 * @brief 触控数据包
 */
typedef struct {
    uint8_t point_num;                  // 当前触摸点数量
    ft6336_point_t points[FT6336_MAX_TOUCH]; // 具体点信息
} ft6336_data_t;

extern ft6336_data_t fp_data;

/* ---------------- 基础接口 ---------------- */
uint8_t ft6336_init(void);
/* ---------------- 逻辑接口 ---------------- */
uint8_t ft6336_read_points(ft6336_data_t *data);
void ft6336_scan(void);

void ft6336_test(void);

#endif