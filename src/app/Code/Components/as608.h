#ifndef __AS608_H
#define __AS608_H

#include <stdint.h>

// AS608 标准参数
#define AS608_HEAD          0xEF01
#define AS608_ADDR          0xFFFFFFFF
#define AS608_PKT_CMD       0x01

// 常用确认码 (Confirmation Code)
#define AS608_OK            0x00
#define AS608_ERR_COMM      0x01 // 收包错误
#define AS608_ERR_NO_FINGER 0x02 // 无手指
#define AS608_ERR_MATCH     0x08 // 指纹不匹配
#define AS608_ERR_NOT_FOUND 0x09 // 未搜索到
#define AS608_ERR_DB_FULL   0x1F // 库满

/* ---------------- 基础接口 ---------------- */
void as608_init(void);

// 核心指令接口 (上层业务组合使用)
uint8_t as608_cmd_get_image(void);
uint8_t as608_cmd_gen_char(uint8_t buffer_id);
uint8_t as608_cmd_match(void);
uint8_t as608_cmd_reg_model(void);
uint8_t as608_cmd_store_char(uint8_t buffer_id, uint16_t page_id);
uint8_t as608_cmd_search(uint8_t buffer_id, uint16_t start_page, uint16_t page_num, uint16_t *id, uint16_t *score);
uint8_t as608_cmd_empty(void);
uint8_t as608_cmd_delete(uint16_t page_id, uint16_t num);

#endif