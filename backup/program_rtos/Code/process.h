#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "main.h"
#include <stdbool.h>

#define PASSWORD_MAX_LEN			8
#define PASSWORD_LEN		5

typedef enum{
	PCD_CHECK_EXIST = 0,
	PCD_ADD_CARD,
	PCD_DELETE_CARD,
	
	PCD_COMPLETE,
	PCD_FAIL
}pcd_flag_t;

typedef enum{
	FP_VERIFY = 0,
	FP_ADD,
	FP_DELETE,
	
	FP_COMPLETE,
	FP_FAIL
}fp_flag_t;

extern volatile uint8_t open_flag;
extern volatile uint8_t pcd_flag;
extern volatile uint8_t fp_flag;

extern uint8_t password[];
extern uint8_t password_len;
extern uint32_t active_start_addr;
extern uint16_t store_cnt;
extern uint16_t fp_id;

void pcd_scan(pcd_flag_t flag);
bool card_exists(uint8_t* target_id);
uint8_t add_card_id(uint8_t* new_id);
bool delete_card_id(uint8_t* target_id);

uint8_t fp_scan(fp_flag_t fp_flag);
uint8_t fp_find_idle_id(uint16_t* id_val);

#endif 

