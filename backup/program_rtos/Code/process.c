/**
  ******************************************************************************
  * @file    process.c
  * @brief   处理门禁系统
  ******************************************************************************
  */

#include "process.h"
#include "RC522.h"
#include "w25qxx.h"
#include "as608.h"
#include <string.h>
#include "dwt.h"
#include "my_ui.h"
#include "log.h"

#include "FreeRTOS.h"
#include "queue.h"

static const char* TAG = "process";

// extern QueueHandle_t xQueue_State;

volatile uint8_t open_flag = 0;
volatile uint8_t pcd_flag = 0;
volatile uint8_t fp_flag = 0;

/*************************************************************/
/*                      密码开门                             */
/*************************************************************/
uint8_t password[PASSWORD_MAX_LEN] = {1,2,3,4,5};	//密码
uint8_t password_len = 5;

/*************************************************************/
/*                      M1卡刷门                             */
/*************************************************************/
 #define ENTRIES_PER_SECTOR		818	//(4096)/5

typedef struct {
    uint8_t status;    // 状态标记:0xFF（空白）、0xAA（有效）、0x55（无效/已删除）
    uint8_t card_id[4];  // 4字节卡号
} card_entry_t;

//扇区前两个字节用于表明自己就是存储卡号的扇区
uint32_t active_start_addr = 0x00;
uint16_t store_cnt = 0;			//已存储的卡数目（保存到内部FLASH）

/**
  * @brief  检测PCD读卡情况
  * @param	void
  * @retval 无
  */
void pcd_scan( pcd_flag_t flag ){
	uint8_t Card_Type[2] = {0x00,0x00};  	//Mifare One(S50)卡
	uint8_t card_id[4];   //卡ID
	uint8_t status;

	static uint8_t add_state_flag = 0;
	static uint8_t del_state_flag = 0;
		
	//按照相应标志位执行代码
	if(flag==PCD_CHECK_EXIST){
		add_state_flag = 0;
		del_state_flag = 0;

		ui_msgbox_info_t info;
		info.ret_to_main = 0;
		info.close_msgbox = 1;
		info.has_close_msgbox = 1;
		info.target_scr = UI_MAIN_SCREEN;

		status = PcdRequest(PICC_REQIDL, Card_Type);//寻卡函数，如果成功返回MI_OK，Card_Typr接收卡片的型号代码

		if(status == MI_OK){
			status = PcdAnticoll(card_id);//防冲撞 如果成功返回MI_OK
			if(status == MI_OK){
				printf("Card Num:%.2x%.2x%.2x%.2x\r\n",card_id[0],card_id[1],card_id[2],card_id[3]);					
			}
			status = PcdSelect(card_id);
			status = PcdHalt();  //卡片进入休眠状态

			if(card_exists(card_id)){
				info.msg_in_box = "Verify Success!";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
			}
			else{
				info.msg_in_box = "Verify Fail!";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
			}
		}
	}
	else if(flag==PCD_ADD_CARD){
		ui_msgbox_info_t info;
		info.ret_to_main = 0;
		info.has_close_msgbox = 1;
		info.target_scr = UI_ADD_CARD_SCREEN;

		if(add_state_flag == 0){
			add_state_flag = 1;
			info.close_msgbox = 0;
			info.msg_in_box = "Please put on the card...";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}

		status = PcdRequest(PICC_REQIDL, Card_Type);//寻卡函数，如果成功返回MI_OK，Card_Typr接收卡片的型号代码
		if(status == MI_OK){
			status = PcdAnticoll(card_id);//防冲撞 如果成功返回MI_OK
			if(status == MI_OK){
				printf("Card Num:%.2x%.2x%.2x%.2x\r\n",card_id[0],card_id[1],card_id[2],card_id[3]);					
			}
			status = PcdSelect(card_id);
			status = PcdHalt();  //卡片进入休眠状态

			uint8_t ret = add_card_id(card_id);
			if(ret == 1){
				add_state_flag = 0;
				info.close_msgbox = 1;
				info.msg_in_box = "Add card success";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				vTaskDelay(pdMS_TO_TICKS(1500));
			}
			else if(ret == 0){
				add_state_flag = 0;
				info.close_msgbox = 1;
				info.msg_in_box = "Add card fail";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				vTaskDelay(pdMS_TO_TICKS(1500));
			}
			else if(ret == 2){
				add_state_flag = 0;
				info.close_msgbox = 1;
				info.msg_in_box = "Add card fail, because sector is filled";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				vTaskDelay(pdMS_TO_TICKS(1500));
			}
		}
	}
	else if(flag==PCD_DELETE_CARD){
		ui_msgbox_info_t info;
		info.ret_to_main = 0;
		info.has_close_msgbox = 1;
		info.target_scr = UI_DELETE_CARD_SCREEN;

		if(del_state_flag == 0){
			del_state_flag = 1;
			info.close_msgbox = 0;
			info.msg_in_box = "Please put on the card...";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}

		status = PcdRequest(PICC_REQIDL, Card_Type);//寻卡函数，如果成功返回MI_OK，Card_Typr接收卡片的型号代码
		if(status == MI_OK){
			status = PcdAnticoll(card_id);//防冲撞 如果成功返回MI_OK
			if(status == MI_OK){
				printf("Card Num:%.2x%.2x%.2x%.2x\r\n",card_id[0],card_id[1],card_id[2],card_id[3]);					
			}
			status = PcdSelect(card_id);
			status = PcdHalt();  //卡片进入休眠状态
			
			bool ret = delete_card_id(card_id);
			if(ret){
				del_state_flag = 0;
				info.close_msgbox = 1;
				info.msg_in_box = "Delete card success";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				vTaskDelay(pdMS_TO_TICKS(1500));
			}
			else{
				del_state_flag = 0;
				info.close_msgbox = 1;
				info.msg_in_box = "Delete card fail";
				xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				vTaskDelay(pdMS_TO_TICKS(1500));
			}
		}
	}
	vTaskDelay(10);
}

/**
  * @brief  检查卡号是否存在
  * @param	target_id 卡号
  * @retval true/false
  */
bool card_exists(uint8_t* target_id) {
    uint32_t current_addr = active_start_addr;
    card_entry_t entry;
    LOG_DEBUG(TAG, "active_start_addr is %ld", active_start_addr);
    
	// 只遍历当前活跃扇区！
    for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {
        W25QXX_BufferRead(&(entry.status), current_addr, 1);
        
        if (entry.status == 0xFF) {
            break; // 遇到空白条码，说明后面都没数据，停止遍历
        }
        else if (entry.status == 0xAA) {
			W25QXX_BufferRead(entry.card_id, current_addr+1, 4);
			if(memcmp(entry.card_id, target_id, 4) == 0){
				printf("卡号匹配成功\r\n");
				return true; // 找到有效卡号
			}
			else{
//				printf("Card Num:%.2x%.2x%.2x%.2x\r\n",entry.card_id[0],entry.card_id[1],entry.card_id[2],entry.card_id[3]);
			}
        }
        current_addr += 5; // 地址递增，指向下一个条目
    }
	printf("卡号未登记\r\n");
    return false;
}

/**
 * @brief 清除扇区垃圾数据
 */
void sector_garbage_clear(void){
	uint32_t old_start_addr = 0;
	if(active_start_addr == 0x0000){
		old_start_addr = active_start_addr;
		active_start_addr = 0x1000;
	}
	else if(active_start_addr == 0x1000){
		old_start_addr = active_start_addr;
		active_start_addr = 0x0000;
	}

	card_entry_t entry;
	uint32_t read_addr = old_start_addr, write_addr = active_start_addr;
	for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {
		W25QXX_BufferRead(&(entry.status), read_addr, 1);
		if ((entry.status) == 0xAA){
			W25QXX_BufferRead(entry.card_id, read_addr+1, 4);
			W25QXX_BufferWrite(&(entry.status), write_addr, 1);
			W25QXX_BufferWrite(entry.card_id, write_addr+1, 4);
			write_addr += 5;
		}
		else if((entry.status) == 0xFF){
			break;
		}
		read_addr += 5;
	}

	// 清理原扇区
	W25QXX_SectorErase(old_start_addr);
	LOG_INFO(TAG, "erase old sector");
	data_flash_write();
}

/**
  * @brief  添加卡号
  * @param	new_id 卡号
  * @retval 0:已存在卡号 1:找到卡号 2:扇区满了
  */
uint8_t add_card_id(uint8_t* new_id){
	//先查找存储中是否存在该卡号
	if(!card_exists(new_id)){
		uint32_t current_addr = active_start_addr;
		card_entry_t entry;

		// 寻找第一个空白位置
		for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {
			W25QXX_BufferRead(&(entry.status), current_addr, 1);
			if (entry.status == 0xFF) {
				// 找到空白位置，准备写入
				entry.status = 0xAA;
				W25QXX_BufferWrite(&(entry.status), current_addr, 1);
				W25QXX_BufferWrite(new_id, current_addr+1, 4);
				printf("登记成功\r\n");
				return 1;
			}
			current_addr += 5;
		}
		printf("扇区满了\r\n");
		
		sector_garbage_clear(); // 触发垃圾回收，腾出空间

		// 重新调用一次，此时就有空间了
		current_addr = active_start_addr;	// active_start_addr 有变化
		for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {
			W25QXX_BufferRead(&(entry.status), current_addr, 1);
			if (entry.status == 0xFF) {
				// 找到空白位置，准备写入
				entry.status = 0xAA;
				W25QXX_BufferWrite(&(entry.status), current_addr, 1);
				W25QXX_BufferWrite(new_id, current_addr+1, 4);
				printf("登记成功\r\n");
				return 1;
			}
			current_addr += 5;
		}
		return 2;
	}
	return 0;
}

/**
  * @brief  删除卡号（只是设置标志而已）
  * @param	target_id 卡号
  * @retval true/false
  */
bool delete_card_id(uint8_t* target_id) {
    uint32_t current_addr = active_start_addr;
    card_entry_t entry;
    
    for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {
        W25QXX_BufferRead(&(entry.status), current_addr, 1);
        
        if (entry.status == 0xFF){
			break; // 到头了，没找到
        }
        // 找到有效且匹配的卡号
        else if (entry.status == 0xAA){
			W25QXX_BufferRead(entry.card_id, current_addr+1, 4);
			if(memcmp(entry.card_id, target_id, 4) == 0) {
				// 关键操作：只修改状态标记为“无效”
				uint8_t invalid_status = 0x55;	//实际上是把剩下的1置零
				// 注意：这是写入操作，不是擦除！只需写入状态字节所在的位置。
				W25QXX_BufferWrite(&(invalid_status), current_addr, 1);
				printf("删除成功\r\n");
				return true; // 删除成功
			}
		}
		current_addr += 5;
    }
	printf("没找到卡号\r\n");
    return false; // 没找到这个卡号
}
	

/**
  * @brief  获取已存储的卡的数量
  * @param	void
  * @retval true/false
  */
uint16_t get_card_num(void){
	
}

///**
//  * @brief  保存卡存储的相关信息到内部flash
//  * @param	void
//  * @retval void
//  */
//void save_card_info(uint16_t cnt, uint32_t base){
//	//存储卡数目
//	card_entry_t entry;
//	
//}

//void renew_card_info(){
//	//已扇区为单位开始查找
//}

/*************************************************************/
/*                      指纹识别                             */
/*************************************************************/
uint16_t fp_id = 0x01;

uint8_t fp_scan(fp_flag_t fp_flag){
	static uint8_t has_chech_idle = 0;	// 防止同一次FP_ADD状态中反复检查空闲ID
	static uint8_t delete_state_flag = 0;	// delete状态下用的标志位
	if(fp_flag == FP_VERIFY){
		has_chech_idle = 0;
		delete_state_flag = 0;
		uint16_t temp_id;
		uint16_t ret = as608_verify_fingerprint(&temp_id);

		ui_msgbox_info_t info;
		info.ret_to_main = 0;
		info.close_msgbox = 1;
		info.has_close_msgbox = 1;
		info.target_scr = UI_MAIN_SCREEN;

		if(ret == 1){
			info.msg_in_box = "Verify Success!";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}
		else if(ret == 2){
			info.msg_in_box = "Verify Fail!";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}
	}
	else if(fp_flag == FP_ADD){
		if(has_chech_idle == 0){
			has_chech_idle = 1;
			if(!fp_find_idle_id(&fp_id)){
				return 0;
			}
			ui_msgbox_info_t info;
			info.ret_to_main = 0;
			info.close_msgbox = 0;
			info.target_scr = UI_ADD_FP_SCREEN;
			info.msg_in_box = "Please put on your finger";
			info.has_close_msgbox = 1;
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}
		
		uint8_t res = as608_add_fingerprint(fp_id);
		ui_msgbox_info_t info;
		info.ret_to_main = 1;
		info.close_msgbox = 1;
		info.has_close_msgbox = 1;
		info.target_scr = UI_ADD_FP_SCREEN;

		if(res == 0){
			LOG_INFO(TAG, "录入指纹成功");
			info.msg_in_box = "Add finger print success!";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
			return 1;
		}
		else if(res == 1){
			LOG_ERROR(TAG, "录入指纹失败");
			info.msg_in_box = "Add finger print fail!";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
			return 0;
		}
		else if(res == 3){
			LOG_ERROR(TAG, "指纹已存在");
			info.msg_in_box = "Finger print has exist!";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}
	}
	else if(fp_flag == FP_DELETE){
		ui_msgbox_info_t info;
		info.ret_to_main = 0;
		info.has_close_msgbox = 1;
		info.target_scr = UI_DELETE_FP_SCREEN;

		if(delete_state_flag == 0){
			delete_state_flag = 1;
			info.close_msgbox = 0;
			info.msg_in_box = "Please put on your finger";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
		}
		
		uint8_t res = as608_delete_fingerprint();
		if(res == 0){
			delete_state_flag = 0;
			info.close_msgbox = 1;
			info.msg_in_box = "Delete finger success";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
			vTaskDelay(1500);
		}
		else if(res == 1){
			delete_state_flag = 0;
			info.close_msgbox = 1;
			info.msg_in_box = "Delete finger fail, maybe no exist";
			xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
			vTaskDelay(1500);
		}
	}
}

/**
  * @brief  寻找空闲ID号
  * @param	void
  * @retval true/false
  */
uint8_t fp_find_idle_id(uint16_t* id_val){
	uint16_t fingerprints_num = as608_find_fingerprints_num();
	for(int i=0x01; i<=fingerprints_num+1; i++){
		if(as608_check_idle_id(i)){
//			printf("%.2x是空的\r\n", i);
			*id_val = i;
			return 1;
		}
	}
	return 0;
}

//void fp_add(void){
//	uint8_t res = as608_add_fingerprint(fp_id);
//	if(res == 0){
//		printf("录入指纹成功\r\n");
//	}
//	else{
//		printf("录入指纹失败\r\n");
//	}
//}

//void fp_delete(void){
//	as608_delete_fingerprint();
//}

void process_init(void){
	//读取相关保存信息
	
	//打印验证
	
	//每次这些变量有发生变化就保存
	
}

/*************************************************************/
/*                      人脸识别（暂无）                     */
/*************************************************************/
