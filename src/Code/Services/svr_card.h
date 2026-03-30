#ifndef __SVR_CARD_H
#define __SVR_CARD_H

#include <stdint.h>

// 定义服务层状态
typedef enum {
    SVR_CARD_OK,
    SVR_CARD_FAIL,
} svr_card_status_t;

svr_card_status_t svr_card_get_id(uint8_t* uid);

#endif