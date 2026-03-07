/**
  ******************************************************************************
  * @file    test.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  */
#include "lcd.h"
#include "touch.h"
#include "w25qxx.h"
#include "test.h"
#include "dwt.h"
#include "RC522.h"
#include "as608.h"
#include "process.h"

#if 0
void DrawTestPage(uint8_t *str)
{
	//绘制固定栏up
	LCD_Clear(WHITE);
	LCD_Fill(0,0,lcddev.width,20,BLUE);
	//绘制固定栏down
	LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
	POINT_COLOR=BLACK;
}

void W25QXX_Test(void){
	//	uint32_t id_ret = W25QXX_ReadID();
//	uint8_t tx_buf[10]={1, 2, 3, 7, 9, 6, 7, 8, 9, 0};
//	uint8_t rx_buf[10]={0};
//	
//	W25QXX_SectorErase(0x000000);
//	W25QXX_BufferWrite(tx_buf, 0x000000, 10);
//	W25QXX_BufferRead(rx_buf, 0x000000, 10);
//	printf(" %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 
//			rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3], rx_buf[4], 
//			rx_buf[5], rx_buf[6], rx_buf[7], rx_buf[8], rx_buf[9]);
}

void Touch_Adjust_Test(void){
	LCD_Init();
	Touch_Init();	//里面启动触摸校准
//	printf("%f, %d, %f, %d\n", Pen_Point.xfac, Pen_Point.xoff, Pen_Point.yfac, Pen_Point.yoff);
	bsp_Delayms(10);
	while(1){
		
		if(Pen_Point.Key_Sta == Key_Down){
			Convert_Pos();		//读取坐标并转换
			Pen_Point.Key_Sta=Key_Up;
			LCD_DrawPoint(Pen_Point.X, Pen_Point.Y);
		}
	}
}

void Touch_Test(void){
	LCD_Init();
	Touch_Init();
	
	while(1){
		if(Pen_Point.Key_Sta == Key_Down){
			uint16_t res = XPT_Read_AD(0xD0);
			Pen_Point.Key_Sta=Key_Up;
			printf("%d\n", res);
		}
	}
	//		if(Pen_Point.Key_Sta == Key_Down){
//			Convert_Pos();
//			Pen_Point.Key_Sta=Key_Up;
//			printf("%d, %d\n", Pen_Point.X0, Pen_Point.Y0);
//			Pen_Point.Key_Sta = Key_Up;
//			uint16_t res = XPT_Read_AD(0xD0);
//			uint16_t res = XPT_Read_XY(0xD0);
//			Pen_Point.Key_Sta=Key_Up;
//			if(res != 0 && res > 100){
//				printf("%d\n", res);
//			}
//		}
}

void LCD_Direction_Test(void){
	LCD_Init();
	LCD_direction(1);
	DrawTestPage(NULL);
	while(1){
		
	}
}

void LCD_Touch_Drawing(void){
	LCD_Init();
	Touch_Init();
	
	while(1){
		if(Pen_Point.Key_Sta == Key_Down){
			Convert_Pos();		//读取坐标并转换
			Pen_Point.Key_Sta=Key_Up;
			LCD_DrawPoint(Pen_Point.X, Pen_Point.Y);
		}
	}	
}

#endif

void DrawTestPage(u8 *str){
	//绘制固定栏up
	LCD_Clear(WHITE);
	LCD_Fill(0,0,lcddev.width,20,BLUE);
	//绘制固定栏down
	LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
	POINT_COLOR=WHITE;
	Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//居中显示
	Gui_StrCenter(0,lcddev.height-18,WHITE,BLUE,"http://www.lcdwiki.com",16,1);//居中显示
	//绘制测试区域
	//LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
}

void Touch_Pen_Test(void)
{
//	u8 i=0,j=0;	 
 	u16 lastpos[2];		//最后一次的数据 
	DrawTestPage("测试12:触摸PEN测试");
	LCD_ShowString(lcddev.width-32,2,16,"RST",1);//显示清屏区域
	POINT_COLOR=RED;//设置画笔蓝色 //清除
	while(1)
	{
		//j++;
		tp_dev.scan();
		//for(t=0;t<CTP_MAX_TOUCH;t++)//最多5点触摸
		//{
			if((tp_dev.sta)&(1<<0))//判断是否有点触摸？
			{
				if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)//在LCD范围内
				{
					if(lastpos[0]==0XFFFF)
					{
						lastpos[0] = tp_dev.x[0];
						lastpos[1] = tp_dev.y[0];
					}
					if(tp_dev.x[0]>(lcddev.width-32)&&tp_dev.y[0]<18)
					{
							//if(j>1) //防止点击一次，多次清屏
							//{
							//	continue;
							//}
							tp_dev.x[0]=0xFFFF;
							tp_dev.x[0]=0xFFFF;
							DrawTestPage("测试12:触摸PEN测试");
							LCD_ShowString(lcddev.width-32,2,16,"RST",1);//显示清屏区域
							POINT_COLOR=RED;//设置画笔蓝色 //清除
					}
					else
					{
							LCD_DrawLine2(lastpos[0],lastpos[1],tp_dev.x[0],tp_dev.y[0],2,RED);//画线
					}
					lastpos[0]=tp_dev.x[0];
					lastpos[1]=tp_dev.y[0];
				}
			}else lastpos[0]=0XFFFF;
	} 
}

void RC522_Test(void){
	unsigned char Card_Type[2] = {0x00,0x00};  	//Mifare One(S50)卡
	unsigned char Card_ID[4];   //卡ID
	unsigned char Card_KEY[6] = {0xff,0xff,0xff,0xff,0xff,0xff};   //默认密钥（A和B一样）
	unsigned char block =0x06;	//要读/写的块地址
	unsigned char Card_Write_Data[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x1F};  //要写如的数据
	unsigned char Card_Read_Data[16];	//要读出的数据
	unsigned char status;	//记录多种状态

	RC522_Init();
	RC522_Rese();
	RC522_Config_Type();
	
	while(1){
		status = PcdRequest(PICC_REQIDL, Card_Type);//寻卡函数，如果成功返回MI_OK，Card_Typr接收卡片的型号代码
		
		if(status == MI_OK)  
		{
			printf("\r\n");
			printf("Card found successfully. Card type:%.2x%.2x\r\n",Card_Type[0],Card_Type[1]);
			status = PcdAnticoll(Card_ID);//防冲撞 如果成功返回MI_OK
			if(status == MI_OK)
			{
				printf("Card Num:%.2x%.2x%.2x%.2x\r\n",Card_ID[0],Card_ID[1],Card_ID[2],Card_ID[3]);					
			}
			else
			{
				printf("Multiple cards detected.\r\n");			
				continue;
			}
			
			status = PcdSelect(Card_ID);  //选卡 如果成功返回MI_OK
			if(status == MI_OK)
			{	
				printf("Card selection successful.\r\n");
			}
			else
			{
				printf("Card selection failed\r\n");
				continue;
			}
			
			status = PcdAuthState(PICC_AUTHENT1A,block,Card_KEY,Card_ID); //验证卡片密码
			if(status == MI_OK)
			{
				printf("successful authentication\r\n");
			}
			else
			{
				printf("authentication failed\r\n");
				continue;
			}
						
			status = PcdWrite(block,Card_Write_Data); //写指定块
			if(status == MI_OK)
			{
				printf("Write specified block successful\r\n");
			}
			else
			{
				printf("Write specified block failed\r\n");
				continue;
			}
			
			status = PcdRead(block,Card_Read_Data); //读指定块
			if(status == MI_OK)
			{
				printf("Read specified block successful");
				printf(", 0x%.2x块的数据为：%.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n",block,*Card_Read_Data,*(Card_Read_Data+1),*(Card_Read_Data+2),*(Card_Read_Data+3),*(Card_Read_Data+4),*(Card_Read_Data+5),*(Card_Read_Data+6),*(Card_Read_Data+7),*(Card_Read_Data+8),*(Card_Read_Data+9),*(Card_Read_Data+10),*(Card_Read_Data+11),*(Card_Read_Data+12),*(Card_Read_Data+13),*(Card_Read_Data+14),*(Card_Read_Data+15));
			}
			else
			{
				printf("Read specified block failed\r\n");
				continue;
			}
			
			status = PcdHalt();  //卡片进入休眠状态
			if(status == MI_OK)
				{	
				printf("Sleeping successfully\r\n");
			}
			else
			{
				printf("Sleeping failed\r\n");
			}
		}	
		else
		{
	//		printf("寻卡失败\r\n");
		}
		HAL_Delay(10);
  }
}

void Receive_Idle_Test(void){
	as608_init();
	
}

void AS608_Test(void){
//	printf("123\r\n");
	as608_init();
	as608_empty_all_fingerprint();
	uint16_t num = as608_find_fingerprints_num();
	if(num == 0){
		as608_add_fingerprint(0x01);
	}
	num = as608_find_fingerprints_num();
	
	while(1){
		HAL_Delay(100);
		receive_flag = 0;
		if(num>0)
			as608_verify_fingerprint(NULL);	
	}
}

void verify_card_test(void){
	RC522_Init();
	RC522_Rese();
	RC522_Config_Type();
//	W25QXX_SectorErase(0x000000);
	uint8_t card_id[4] = {0};
	
	while(1){
		pcd_scan(PCD_CHECK_EXIST);
//		pcd_scan(ADD_CARD);
	}
}
