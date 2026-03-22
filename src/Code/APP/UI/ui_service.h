#ifndef __UI_SERVICE_H
#define __UI_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

// 有新的服务类型就往这边加
typedef enum{
    UI_SERVICE_LOAD_PWD = 0,
    UI_SERVICE_SAVE_PWD,
    UI_SERVICE_SET_FINGER_VERIFY,
    UI_SERVICE_SET_FINGER_ADD,
    UI_SERVICE_SET_FINGER_DEL,
    UI_SERVICE_SET_CARD_VERIFY,
    UI_SERVICE_SET_CARD_ADD,
    UI_SERVICE_SET_CARD_DEL,
    UI_SERVICE_WAIT,
    UI_SERVICE_OPEN_DOOR,
}ui_service_type_t;

typedef struct {
    uint8_t type;
    char content[32];
    uint32_t duration;
} ui_toast_request_t;

bool ui_service_require(ui_service_type_t type, uint32_t* param);
void ui_service_init(void);

#endif