#ifndef MY_LVGL_H
#define MY_LVGL_H

#include <stdint.h>
#include "lvgl/lvgl.h"

// 定义页面类型枚举
typedef enum {
    MAIN_PAGE,
    PASSWORD_PAGE,
    VERIFY_PAGE,
    PAGE_COUNT
} page_t;

//具体页面位置枚举
typedef enum {
    IDLE,
    ENTER_PASSWORD,
    CHANGE_PASSWORD,
    ADD_FINGER,
    ADD_CARD,
    DELETE_FINGER,
    DELETE_CARD
} task_t;

// 页面管理器结构
typedef struct {
    lv_obj_t* screens[PAGE_COUNT];
    page_t current_page;
} page_manager_t;

//页面详情结构体
typedef struct {
    task_t task;
    uint8_t status;
} page_info_t;

void page_manager_init(lv_obj_t* parent);

#endif
