#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "ui_service.h"
#include "ui_router.h"

void ui_init(void){
    lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
//    ui_service_init();
    ui_router_navigate(UI_PAGE_MAIN, NULL);
}