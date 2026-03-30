#include "svr_card.h"
#include "rc522.h"

static struct{
    void (*init)(void);
    rc522_status_t (*get_id)(uint8_t* uid);
    void (*halt)(void);
}driver = {
    .init = rc522_init,
    .get_id = rc522_identify_card,
    .halt = rc522_halt,
};

void svr_card_init(void){
    driver.init();
}

/**
 * @brief 寻卡并获取UID
 * @param uid 存储获取到的4字节UID
 * @return svr_card_status_t 
 */
svr_card_status_t svr_card_get_id(uint8_t* uid){
    rc522_status_t res = driver.get_id(uid);
    if(res == RC522_OK){
        driver.halt();
        return SVR_CARD_OK;
    }
    else if(res == RC522_NOTAG){
        return SVR_CARD_FAIL;
    }
    else{
        driver.halt();
        return SVR_CARD_FAIL;
    }
}

