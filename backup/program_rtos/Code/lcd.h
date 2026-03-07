#ifndef __LCD_H__
#define __LCD_H__

#include "main.h"
#include "lvgl.h"

#define u8	uint8_t
#define u16 uint16_t

/*************引脚分配*****************/
#define LCD_CS_PORT		GPIOA
#define LCD_CS_PIN		GPIO_PIN_3
#define LCD_RS_PORT		GPIOA
#define LCD_RS_PIN		GPIO_PIN_2
#define LCD_CLK_PORT		GPIOA
#define LCD_CLK_PIN		GPIO_PIN_5
#define LCD_MISO_PORT		GPIOA
#define LCD_MISO_PIN		GPIO_PIN_6
#define LCD_MOSI_PORT		GPIOA
#define LCD_MOSI_PIN		GPIO_PIN_7
#define LCD_RST_PORT	GPIOA
#define LCD_RST_PIN		GPIO_PIN_4
#define LCD_LED_PORT	GPIOA
#define LCD_LED_PIN		GPIO_PIN_1	//背光控制引脚

/*************************************/

#define LCD_LED_ON		HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_SET);
#define LCD_LED_OFF		HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_RESET);

#define GPIO_TYPE GPIOA		 

#define	LCD_CS_SET  GPIO_TYPE->BSRR = LCD_CS_PIN    //片选端口  	PB11
#define	LCD_RS_SET	GPIO_TYPE->BSRR = LCD_RS_PIN    //数据/命令  PB10	  
#define	LCD_RST_SET	GPIO_TYPE->BSRR = LCD_RST_PIN    //复位			PB12
		    
#define	LCD_CS_CLR  GPIO_TYPE->BSRR = (LCD_CS_PIN<<16)     //片选端口  	PB11
#define	LCD_RS_CLR	GPIO_TYPE->BSRR = (LCD_RS_PIN<<16)     //数据/命令  PB10	 
#define	LCD_RST_CLR	GPIO_TYPE->BSRR = (LCD_RST_PIN<<16)    //复位			  PB12	

#define USE_HORIZONTAL  	 0//定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转
  
//定义LCD的尺寸
#define LCD_W 240
#define LCD_H 320

//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				//LCD ID
	u8  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	 wramcmd;		//开始写gram指令
	u16  rramcmd;   	//开始读gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令	 
}_lcd_dev; 	

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数

//TFTLCD部分外要调用的函数		   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色   

extern SPI_HandleTypeDef hspi1;

//画笔颜色
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //棕色
#define BRRED 			0XFC07 //棕红色
#define GRAY  			0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	0X841F //浅绿色
#define LIGHTGRAY     0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 		0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE      	0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)
	    															  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//画点
u16  LCD_ReadPoint(u16 x,u16 y); //读点
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);

u8 LCD_RD_DATA(void);//读取LCD数据									    
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
void LCD_WR_DATA(u8 data);
u8 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);		   
u16 LCD_BGR2RGB(u16 c);
void LCD_SetParam(void);
void Lcd_WriteData_16Bit(u16 Data);
void LCD_direction(u8 direction );
u16 LCD_Read_ID(void);		

/******应用层*******/
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
void LCD_ColorFill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t* color_p);
void LCD_Fill_LVGL(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, lv_color_t *color_p);

void LCD_DrawLine2(u16 x1, u16 y1, u16 x2, u16 y2, u16 size, u16 color);
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode);
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
#endif  