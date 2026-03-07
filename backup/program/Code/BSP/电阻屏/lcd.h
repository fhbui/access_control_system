#ifndef __LCD_H__
#define __LCD_H__

#include "main.h"

/*************引脚分配*****************/
#define LCD_CS_PORT		GPIOD
#define LCD_CS_PIN		GPIO_PIN_7

#define LCD_RS_PORT		GPIOD
#define LCD_RS_PIN		GPIO_PIN_13

#define LCD_WR_PORT		GPIOD
#define LCD_WR_PIN		GPIO_PIN_5

#define LCD_RD_PORT		GPIOD
#define LCD_RD_PIN		GPIO_PIN_4

#define LCD_RST_PORT	GPIOD
#define LCD_RST_PIN		GPIO_PIN_6

#define LCD_BL_PORT		GPIOD
#define LCD_BL_PIN		GPIO_PIN_8

#define LCD_DB_PORT		GPIOE
#define LCD_DB0_PORT	GPIOE
#define LCD_DB0_PIN		GPIO_PIN_0
#define LCD_DB1_PORT	GPIOE
#define LCD_DB1_PIN		GPIO_PIN_1
#define LCD_DB2_PORT	GPIOE
#define LCD_DB2_PIN		GPIO_PIN_2
#define LCD_DB3_PORT	GPIOE
#define LCD_DB3_PIN		GPIO_PIN_3
#define LCD_DB4_PORT	GPIOE
#define LCD_DB4_PIN		GPIO_PIN_4
#define LCD_DB5_PORT	GPIOE
#define LCD_DB5_PIN		GPIO_PIN_5
#define LCD_DB6_PORT	GPIOE
#define LCD_DB6_PIN		GPIO_PIN_6
#define LCD_DB7_PORT	GPIOE
#define LCD_DB7_PIN		GPIO_PIN_7
#define LCD_DB8_PORT	GPIOE
#define LCD_DB8_PIN		GPIO_PIN_8
#define LCD_DB9_PORT	GPIOE
#define LCD_DB9_PIN		GPIO_PIN_9
#define LCD_DB10_PORT	GPIOE
#define LCD_DB10_PIN	GPIO_PIN_10
#define LCD_DB11_PORT	GPIOE
#define LCD_DB11_PIN	GPIO_PIN_11
#define LCD_DB12_PORT	GPIOE
#define LCD_DB12_PIN	GPIO_PIN_12
#define LCD_DB13_PORT	GPIOE
#define LCD_DB13_PIN	GPIO_PIN_13
#define LCD_DB14_PORT	GPIOE
#define LCD_DB14_PIN	GPIO_PIN_14
#define LCD_DB15_PORT	GPIOE
#define LCD_DB15_PIN	GPIO_PIN_15

//PE0~15,作为数据线
#define DATAOUT(x) 	LCD_DB_PORT->ODR=x; //数据输出
#define DATAIN     	LCD_DB_PORT->IDR;   //数据输入

//定义LCD的尺寸
#define LCD_W 240
#define LCD_H 320

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

#define LCD_LED_ON		HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);
#define LCD_LED_OFF		HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET);

//GPIO置位（拉高），BSRR寄存器低16位设置引脚高电平
#define	LCD_CS_SET 	LCD_CS_PORT->BSRR = LCD_CS_PIN
#define LCD_RS_SET	LCD_RS_PORT->BSRR = LCD_RS_PIN
#define LCD_RST_SET	LCD_RST_PORT->BSRR = LCD_RST_PIN
#define LCD_WR_SET	LCD_WR_PORT->BSRR = LCD_WR_PIN
#define LCD_RD_SET	LCD_RD_PORT->BSRR = LCD_RD_PIN

//GPIO复位（拉低），BSRR寄存器高16位设置引脚低电平
#define	LCD_CS_RESET 	LCD_CS_PORT->BSRR = (LCD_CS_PIN << 16)
#define LCD_RS_RESET	LCD_RS_PORT->BSRR = (LCD_RS_PIN << 16)
#define LCD_RST_RESET	LCD_RST_PORT->BSRR = (LCD_RST_PIN << 16)
#define LCD_WR_RESET	LCD_WR_PORT->BSRR = (LCD_WR_PIN << 16)
#define LCD_RD_RESET	LCD_RD_PORT->BSRR = (LCD_RD_PIN << 16)

//用户配置
#define USE_HORIZONTAL  	0 //定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转
#define USE_VERTICAL  	    1 
#define LCD_USE8BIT_MODEL   0	//定义数据总线是否使用8位模式 0,使用16位模式.1,使用8位模式

//LCD重要参数集
typedef struct  
{										    
	uint16_t 	width;			//LCD 宽度
	uint16_t 	height;			//LCD 高度
	uint16_t 	id;				  //LCD ID
	uint8_t  	dir;			  //横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	wramcmd;	//开始写gram指令
	uint16_t  	rramcmd;   //开始读gram指令
	uint16_t  	setxcmd;		//设置x坐标指令
	uint16_t  	setycmd;		//设置y坐标指令	 
}_lcd_dev; 	

extern _lcd_dev lcddev;
extern uint16_t  POINT_COLOR;//默认黑色  
extern uint16_t  BACK_COLOR; //背景颜色.默认为白色
	    															  
void LCD_Init(void);
void LCD_write(uint16_t VAL);
uint16_t LCD_read(void);
void LCD_Clear(uint16_t Color);	 
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawPoint(uint16_t x,uint16_t y);//画点
uint16_t  LCD_ReadPoint(uint16_t x,uint16_t y); //读点	   
void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);
uint16_t LCD_RD_DATA(void);//读取LCD数据								    
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
void LCD_WR_REG(uint16_t data);
void LCD_WR_DATA(uint16_t data);
void LCD_ReadReg(uint16_t LCD_Reg,uint8_t *Rval,int n);
void LCD_WriteRAM_Prepare(void);
void LCD_ReadRAM_Prepare(void);   
void Lcd_WriteData_16Bit(uint16_t Data);
uint16_t Lcd_ReadData_16Bit(void);
void LCD_direction(uint8_t direction );
uint16_t Color_To_565(uint8_t r, uint8_t g, uint8_t b);
uint16_t LCD_Read_ID(void);

void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void LCD_ColorFill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t* color_p);
void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend);
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r);
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num, uint8_t size, uint8_t mode);
void LCD_ShowString(uint16_t x,uint16_t y,uint8_t size, uint8_t *p,uint8_t mode);
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);


#endif 

