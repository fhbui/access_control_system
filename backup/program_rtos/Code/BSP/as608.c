#include "as608.h"
#include "dwt.h"
#include <string.h>

#define use_rtos	1

#if use_rtos==1
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "my_ui.h"
#include "log.h"

extern SemaphoreHandle_t xSemaphore_FPflag;
// extern QueueHandle_t xQueue_State;
#endif

#define EXIT	0xFF
#define RX_BUF_SIZE		512

#define CLEAR_BUFFER			NULL
#define OPEN_USART2_RECEIVE		SET_BIT(huart2.Instance->CR1, USART_CR1_RE);	\
								while(receive_flag != 1);	\
								receive_flag = 0;
#define CLOSE_USART2_RECEIVE	CLEAR_BIT(huart2.Instance->CR1, USART_CR1_RE);	//关闭接收

static const char* TAG = "as608";

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

uint8_t as608_receive_data[RX_BUF_SIZE] = {0};		//串口接收的数据
uint8_t check_head[2] = {0xEF, 0x01};

uint16_t id_index = 0x01;		//ID号索引
uint16_t fragmented_num = 0;		//零碎空号数目
uint16_t fragmented_index[1000] = {0};	//存储删除后的存在在中间部分的空闲索引

volatile uint8_t finger_status = FINGER_NO_EXIST;		//要在外部中断中设置
volatile uint8_t receive_flag = 0;

/***************************************************************************
描述: 初始化AS608（串口为了不定长，有必要配置DMA+IDLE）
****************************************************************************/
void as608_init(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	HAL_NVIC_SetPriority(EXTI4_IRQn, 8, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	    
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 57600;	//57600
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	
	hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

	__HAL_LINKDMA(&huart2,hdmarx,hdma_usart2_rx);

	//启动接收
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE); // 手动使能空闲中断
	HAL_UART_Receive_DMA(&huart2, as608_receive_data, RX_BUF_SIZE); // 启动DMA，等待最多RX_BUF_SIZE个字节
}

void HAL_UART_RxIdleCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART2) {
		if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)){
			// 禁用DMA（防止处理过程中被修改）
			//__HAL_DMA_DISABLE(huart->hdmarx);
			HAL_UART_DMAStop(huart);	//用上面那个有问题，用HAL库对外接口会更加稳定
			
			uint16_t recv_len = RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
			receive_flag = 1;
			
			// 重新设置DMA计数器，准备下一次接收（原来用的__HAL，但是不能正常开启DMA）
			HAL_UART_Receive_DMA(huart, as608_receive_data, RX_BUF_SIZE);
			
			// 清除IDLE中断标志位
			__HAL_UART_CLEAR_IDLEFLAG(huart);
		}
    }
}

/***************************************************************************
描述: 发送包头
****************************************************************************/
static void as608_send_head(void)
{
	uint8_t tx_data = AS608_HEAD>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (AS608_HEAD&0xFF);
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	
//	USART2_SendData(AS608_HEAD>>8);
//	USART2_SendData(AS608_HEAD);
}

/***************************************************************************
描述: 发送芯片地址
****************************************************************************/
static void as608_send_address(void)
{
	uint8_t tx_data = AS608_ADDR>>24;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (AS608_ADDR>>16)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (AS608_ADDR>>8)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (AS608_ADDR)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	
//	USART2_SendData(AS608_ADDR>>24);
//	USART2_SendData(AS608_ADDR>>16);
//	USART2_SendData(AS608_ADDR>>8);
//	USART2_SendData(AS608_ADDR);
}

/***************************************************************************
描述: 发送包标识
****************************************************************************/
static void as608_send_logo(uint8_t logo)
{
	uint8_t tx_data = logo;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	
//	USART2_SendData(logo);
}

/***************************************************************************
描述: 发送包长度
****************************************************************************/
static void as608_send_length(uint16_t length)
{
	uint8_t tx_data = length>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (length)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);	
	
//	USART2_SendData(length>>8);
//	USART2_SendData(length);
}

/***************************************************************************
描述: 发送指令码
****************************************************************************/
static void as608_send_cmd(uint8_t cmd)
{
	uint8_t tx_data = cmd;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);	
	
//	USART2_SendData(cmd);
}


/***************************************************************************
描述: 发送校验和
****************************************************************************/
static void as608_send_checksum(uint16_t checksum)
{
	uint8_t tx_data = checksum>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (checksum)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	
//	USART2_SendData(checksum>>8);
//	USART2_SendData(checksum);
}

/***************************************************************************
描述: 发送BufferID（特征缓冲区号）
****************************************************************************/
static void as608_send_BufferID(uint8_t BufferID)
{
	uint8_t tx_data = BufferID;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);	
	
//	USART2_SendData(BufferID);
}

/***************************************************************************
描述: 发送StartPage
****************************************************************************/
static void as608_send_StartPage(uint16_t StartPage)
{
	uint8_t tx_data = StartPage>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (StartPage)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);	
	
//	USART2_SendData(StartPage>>8);
//	USART2_SendData(StartPage);
}

/***************************************************************************
描述: 发送PageNum
****************************************************************************/
static void as608_send_PageNum(uint16_t PageNum)
{
	uint8_t tx_data = PageNum>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (PageNum)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);		
	
//        USART2_SendData(PageNum>>8);
//        USART2_SendData(PageNum);
}

/***************************************************************************
描述: 发送PageID号
****************************************************************************/
static void as608_send_PageID(uint16_t PageID)
{
	uint8_t tx_data = PageID>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (PageID)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);		
	
//	USART2_SendData(PageID>>8);
//	USART2_SendData(PageID);
}

/***************************************************************************
描述: 发送个数
****************************************************************************/
static void as608_send_N(uint16_t N)
{
	uint8_t tx_data = N>>8;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);
	tx_data = (N)&0xFF;
	HAL_UART_Transmit(&huart2, &tx_data, 1, 100);		
	
//	USART2_SendData(N>>8);
//	USART2_SendData(N);
}

/***************************************************************************
描述: 获取手指指纹图像 存放于图像缓冲区
参数: 无                                         指令代码:02H
返回: 00H: 录入指纹成功                01H:收包错误        02H:无手指        03H:录入不成功
****************************************************************************/
uint8_t PS_GetImage(void)
{        
	uint8_t result;            //存放结果

	CLEAR_BUFFER;             //清空缓冲区
	
	as608_send_head();                        //发送包头
	as608_send_address();                //发送芯片地址
	as608_send_logo(0x01);                //发送包标识
	as608_send_length(0x03);        //发送包长度
	as608_send_cmd(0x01);                //发送指令码
	as608_send_checksum(0x05);        //发送校验和
	OPEN_USART2_RECEIVE;                //开启串口接收

	result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;
}

/***************************************************************************
描述: 生成特征
参数: BufferID(特征缓冲区号) 指令代码:02H 
          CharBuffer1 的 BufferID: 01H  CharBuffer2的 BufferID: 02H
返回: 00H: 生成特征成功                01H:收包错误        06H:指纹图像太乱生成失败
          07H: 特征太少                        15H:图像缓冲区没图像
****************************************************************************/
uint8_t PS_GenChar(uint8_t BufferID)
{
	uint16_t checksum;                                //存放校验和
	uint8_t result;                                        //存放结果

	CLEAR_BUFFER;                                        //清空缓冲区
	as608_send_head();                                //发送包头
	as608_send_address();                        //发送芯片地址
	as608_send_logo(0x01);                        //发送包标识
	as608_send_length(0x04);                //发送包长度
	as608_send_cmd(0x02);                        //发送指令码
	as608_send_BufferID(BufferID);        //发送BufferID
	checksum = 0x01 + 0x04 + 0x02 + BufferID;        
	as608_send_checksum(checksum);        //发送校验和
	OPEN_USART2_RECEIVE;                        //开启串口接收

	result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;
}

/***************************************************************************
描述: 精确比对两枚指纹特征 
参数: 无                                        指令代码:03H 
返回: 00H: 指纹匹配成功                08H:指纹不匹配                01H:收包错误
****************************************************************************/
uint8_t PS_Match(void)
{
	uint8_t result;                                        //存放结果
	
    CLEAR_BUFFER;                                        //清空缓冲区
	as608_send_head();                                //发送包头
	as608_send_address();                        //发送芯片地址
	as608_send_logo(0x01);                        //发送包标识
	as608_send_length(0x03);                //发送包长度
	as608_send_cmd(0x03);                        //发送指令码
	as608_send_checksum(0x07);                //发送校验和
    OPEN_USART2_RECEIVE;                        //开启串口接收

	result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;
}

/***************************************************************************
描述: 合并特征 将CharBuffer1 和 CharBuffer2 中的 特征合并生成模板 
          结果存在CharBuffer1 和 CharBuffer2        
参数: 无                                指令代码:05H 
返回: 00H: 合并成功                01H:收包错误        0AH:合并失败(两枚手指不是同一个)
****************************************************************************/
uint8_t PS_RegModel(void)
{
    uint8_t result;                                        //存放结果

	CLEAR_BUFFER;                                        //清空缓冲区
	as608_send_head();                                //发送包头
	as608_send_address();                        //发送芯片地址
	as608_send_logo(0x01);                        //发送包标识
	as608_send_length(0x03);                //发送包长度
	as608_send_cmd(0x05);                        //发送指令码
	as608_send_checksum(0x09);                //发送校验和
	OPEN_USART2_RECEIVE;                        //开启串口接收

	result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;                
}

/***************************************************************************
描述: 储存模板 将CharBuffer1和CharBuffer2的模板文件存到PageID号Flash数据库位置 
参数: BufferID:        01H/02H                PageID:指纹库位置号                指令代码:06H 
返回: 00H: 储存成功        01H:收包错误 0BH:PageID超出指纹库范围 18H:写Flash出错
****************************************************************************/
uint8_t PS_StoreChar(uint8_t BufferID,uint16_t PageID)
{
	uint16_t checksum;                                //存放校验和
	uint8_t result;                                        //存放结果

	CLEAR_BUFFER;                                        //清空缓冲区
	as608_send_head();                                //发送包头
	as608_send_address();                        //发送芯片地址
	as608_send_logo(0x01);                        //发送包标识
	as608_send_length(0x06);                //发送包长度
	as608_send_cmd(0x06);                        //发送指令码
	as608_send_BufferID(BufferID);        //发送BufferID
	as608_send_PageID(PageID);                //发送指纹库位置号        
	checksum = 0x01+0x06+0x06+BufferID+PageID;
	as608_send_checksum(checksum);        //发送校验和
	OPEN_USART2_RECEIVE;                        //开启串口接收

	result = as608_detection_data(300,0);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;                
}

/***************************************************************************
描述: 删除模板  删除Flash数据库中指定的ID号开始的N个指纹模板
参数: PageID:指纹库模板号 N:删除的模板个数                指令代码:0CH 
返回: 00H: 删除模板成功                01H:收包错误         10H:删除模板失败
****************************************************************************/
uint8_t PS_DeletChar(uint16_t PageID,uint16_t N)
{
	uint16_t checksum;                                //存放校验和
	uint8_t result;                                        //存放结果

	CLEAR_BUFFER;                                        //清空缓冲区
	as608_send_head();                                //发送包头
	as608_send_address();                        //发送芯片地址
	as608_send_logo(0x01);                        //发送包标识
	as608_send_length(0x07);                //发送包长度
	as608_send_cmd(0x0C);                        //发送指令码
	as608_send_PageID(PageID);                //发送指纹库位置号
	as608_send_N(N);                                //发送删除模板的个数
	
	checksum = 0x01+0x07+0x0C+PageID+N;
	as608_send_checksum(checksum);                //发送校验和
	OPEN_USART2_RECEIVE;                        //开启串口接收        

	result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;                        
}

/***************************************************************************
描述: 清空Flash数据库中所有指纹模板 
参数: 无                                指令代码:0DH 
返回: 00H: 清空成功        01H:收包错误         11H:清空失败
****************************************************************************/
uint8_t PS_Empty(void)
{
        uint8_t result;                                        //存放结果

        CLEAR_BUFFER;                                        //清空缓冲区
        as608_send_head();                                //发送包头
        as608_send_address();                        //发送芯片地址
        as608_send_logo(0x01);                        //发送包标识
        as608_send_length(0x03);                //发送包长度
        as608_send_cmd(0x0D);                        //发送指令码
        as608_send_checksum(0X11);                //发送校验和
        OPEN_USART2_RECEIVE;                        //开启串口接收        

        result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
        if(result == 0XFF)        result = 0x01;        

        return result;                        
}

/***************************************************************************
描述: 高速搜索 以CharBuffer1或CharBuffer2的特征文件高速搜索整个或者部分指纹库 
参数: BufferID:        01H/02H          StartPage:起始页         PageNum:页数        指令代码:1BH 
      ID: 存放搜索到的指纹ID号 否则为0
返回: 确认字00H: 搜索到        01H:收包错误 09H:没有搜索到(页码就是0) 和 对应页码
****************************************************************************/
uint8_t PS_HighSpeedSearch(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,uint16_t *ID)
{
        uint16_t checksum;                                //存放校验和
        uint8_t result;                                        //存放结果

        CLEAR_BUFFER;                                        //清空缓冲区
        as608_send_head();                                //发送包头
        as608_send_address();                        //发送芯片地址
        as608_send_logo(0x01);                        //发送包标识
        as608_send_length(0x08);                //发送包长度
        as608_send_cmd(0x1B);                        //发送指令码
        as608_send_BufferID(BufferID);        //发送BufferID
        as608_send_StartPage(StartPage);//发送起始页
        as608_send_PageNum(PageNum);        //发送页数        
        checksum = 0x01+0x08+0x1B+BufferID+StartPage+PageNum;
        as608_send_checksum(checksum);        //发送校验和
        OPEN_USART2_RECEIVE;                        //开启串口接收        

        result = as608_detection_data(300,&ID);        //检测指纹模块数据 3秒时间
        if(result == 0XFF)        result = 0x01;        

        return result;                        
}

/***************************************************************************
描述: 搜索指纹                                                                 指令代码:04H 
参数: BufferID:        01H/02H         StartPage:起始页        PageNum:页数 
          ID: 存放搜索到的指纹ID号 否则为0
返回: 00H: 搜索到        09H:没有搜索到                 01H:收包错误
****************************************************************************/
uint8_t PS_Search(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,uint16_t *ID)
{
        uint16_t checksum;                                //存放校验和
        uint8_t result;                                        //存放结果
        
        CLEAR_BUFFER;                                        //清空缓冲区
        as608_send_head();                                //发送包头
        as608_send_address();                        //发送芯片地址
        as608_send_logo(0x01);                        //发送包标识
        as608_send_length(0x08);                //发送包长度
        as608_send_cmd(0x04);                        //发送指令码
        as608_send_BufferID(BufferID);        //发送BufferID
        as608_send_StartPage(StartPage);//发送起始页
        as608_send_PageNum(PageNum);        //发送页数
        checksum = 0x01+0x08+0x04+BufferID+StartPage+PageNum;
        as608_send_checksum(checksum);        //发送校验和
        OPEN_USART2_RECEIVE;                        //开启串口接收                
        
        result = as608_detection_data(300,&ID);        //检测指纹模块数据 3秒时间
        if(result == 0XFF)        result = 0x01;        

        return result;        
}

/***************************************************************************
描述: 读取模板个数 
参数: NUM:个数会保存在NUM中                        指令代码:1DH 
返回: 00H: 注册成功        01H:收包错误         NUM也会返回
****************************************************************************/
uint8_t PS_ValidTempleteNum(uint16_t *NUM)
{
        uint8_t result;                                        //存放结果

        CLEAR_BUFFER;                                        //清空缓冲区
        as608_send_head();                                //发送包头
        as608_send_address();                        //发送芯片地址
        as608_send_logo(0x01);                        //发送包标识
        as608_send_length(0x03);                //发送包长度
        as608_send_cmd(0x1D);                        //发送指令码
        as608_send_checksum(0X21);                //发送校验和
        OPEN_USART2_RECEIVE;                        //开启串口接收        

        result = as608_detection_data(300,&NUM);        //检测指纹模块数据 3秒时间
        if(result == 0XFF)        result = 0x01;        

        return result;                        
}

/***************************************************************************
描述: 读出模块 
参数: NUM:个数会保存在NUM中                        指令代码:1DH 
返回: 00H: 注册成功        01H:收包错误         0cH:读出错误或模板无效
****************************************************************************/
uint8_t PS_LoadChar(uint8_t BufferID,uint16_t PageID){
	uint16_t checksum;       //存放校验和
	uint8_t result;          //存放结果
	
	CLEAR_BUFFER;            //清空缓冲区
	as608_send_head();       //发送包头
	as608_send_address();    //发送芯片地址
	as608_send_logo(0x01);   //发送包标识
	as608_send_length(0x06);     //发送包长度
	as608_send_cmd(0x07);                        //发送指令码
	as608_send_BufferID(BufferID);        //发送BufferID
	as608_send_PageID(PageID);
	checksum = 0x01+0x06+0x07+BufferID+PageID;;
	as608_send_checksum(checksum);        //发送校验和
	OPEN_USART2_RECEIVE;                        //开启串口接收                
	
	result = as608_detection_data(300,NULL);        //检测指纹模块数据 3秒时间
	if(result == 0XFF)        result = 0x01;        

	return result;   	
}

/***************************************************************************
描述: 检测串口返回的数据
参数: wait_time:等待的时间 一次:10ms        
ID_OR_NUM: 如果不是搜索指令或者查找模板个数设置为NULL 否则存放ID号或者NUM个数        
返回: 无效数据返回0XFF  否则返回结果
****************************************************************************/
uint8_t as608_detection_data(uint16_t wait_time,uint16_t **ID_OR_NUM)
{
	volatile char *data;
	uint8_t result = 0XFF;        //存放结果

	while(wait_time--)
	{        
		bsp_Delayms(10);
		//匹配数据帧头
		data = strstr((char *)as608_receive_data,(char *)check_head);	//寻找check_head在as608_receive_data第一次出现的位置
		if(data != NULL)
		{
			result = as608_detection_checknum(data,&*ID_OR_NUM);        
			break;
		}        
	}
	CLOSE_USART2_RECEIVE;                        //禁止串口接收
	return result;        
}

/***************************************************************************
描述: 校验数据是否正确
参数: data:数据包                
ID_OR_NUM: 如果不是搜索指令或者查找模板个数设置为NULL 否则存放ID号或者NUM个数        
返回: 数据错误返回0XFF   否则 返回接收结果
****************************************************************************/
uint8_t as608_detection_checknum(char *data , uint16_t **ID_OR_NUM)
{
	//包标识位置:6        包长度位置:7、8
	uint8_t packet_length;        //包长度
	uint8_t checksum1 = 0;                //数据包校验和
	uint8_t checksum2 = 0;                //计算出的校验和
	uint8_t i;

	packet_length = (data[7]*10) + data[8];        //获取数据包长度
	if(packet_length == 0) return 0XFF;                //如果无长度 返回错误
	
	checksum1 = (data[6+packet_length+1]*10) + data[6+packet_length+2];        //数据包校验和

	//自己校验的校验和
	for(i=0;i<packet_length+1;i++)
	{
		checksum2 += data[i+6];
	}
	//匹配校验和是否准确
	if(checksum1 == checksum2)
	{
		//如果是搜索指令 ID进行保存 如果是查找模板个数 NUM进行保存
		if(*ID_OR_NUM != NULL)
				**ID_OR_NUM = (data[10]*10) + data[11];        //获取指纹ID号/模板个数

		printf("数据校验和匹配 数据位:%#X\r\n",data[9]);
		return data[9];                //返回结果
			
	}
	printf("数据校验和错误 数据位:%#X\r\n\r\n",data[9]);
	return 0XFF;
}

/***************************************************************************
描述: 检测手指是否在模块中
参数: wait_time:等待的时间 一次:10ms
返回: 0:手指在         1:不在
****************************************************************************/
uint8_t as608_detection_finger(uint16_t wait_time)	//注意如果不是中断直接修改这个status，这个延迟可能会不合理
{
//	finger_status = FINGER_NO_EXIST;
	//检测手指是否在模块中

#if use_rtos==0	
	while(wait_time--)
	{
		bsp_Delayms(10);
		if(finger_status == FINGER_EXIST)
		{
			finger_status = FINGER_NO_EXIST;
			return 0;
		}
	}
	return 1;
#else
	if(finger_status != FINGER_EXIST && xSemaphoreTake(xSemaphore_FPflag, wait_time*10)==pdTRUE){
		finger_status = FINGER_EXIST;
	}
	if(finger_status == FINGER_EXIST){
		finger_status = FINGER_NO_EXIST;
		return 0;
	}
	return 1;
#endif
}

/***************************************************************************
描述: 添加指纹 函数
返回: 0: 录入指纹成功        1: 录入指纹失败		2: 未检测到指纹		3: 指纹已存在
注意: PageID不要给0x00，后续检验代码当ID==0x00会返回检验错误
****************************************************************************/
uint8_t as608_add_fingerprint(uint16_t PageID)
{
	uint8_t result;                                        //录入的结果
	uint8_t add_stage = 1;                        //录入的阶段
	int8_t temp = 0;
	uint16_t ID = 0;

	ui_msgbox_info_t info;
	info.ret_to_main = 0;
	info.close_msgbox = 0;
	info.target_scr = UI_ADD_FP_SCREEN;

	static uint8_t queue_send_flag = 1;
	
	while(add_stage != EXIT)
	{
		switch(add_stage)
		{
			//第一阶段 获取第一次指纹图像 并且生成特征图
			case 1:{
				// printf("请放置手指\r\n");
				if(queue_send_flag == 0){
					LOG_DEBUG(TAG, "queue_send_flag == 0");
					queue_send_flag = 1;
					info.msg_in_box = "Please put on your finger";
					info.has_close_msgbox = 1;
					xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				}

				if(as608_detection_finger(100)){//等待手指按下
					return 2;
				}                                
				result = PS_GetImage();        //获取指纹图像
				if(result){
					return 1;
				}
				result = PS_GenChar(CharBuffer1);//生成特征图        
				if(result){
					return 1;
				}
				add_stage = 2;                //跳转到第二阶段
				break;
			}
			//第二阶段 获取第二次指纹图像 并且生成特征图
			case 2:
				// printf("请再放置手指\r\n");
				if(queue_send_flag == 1){
					queue_send_flag = 2;
					info.msg_in_box = "Please put on again";
					info.has_close_msgbox = 1;
					xQueueSend(queue_msgbox_info, &info, portMAX_DELAY);
				}

				if(as608_detection_finger(800)){//等待手指按下     
					goto return_2;
				}
				result = PS_GetImage();        //获取指纹图像
				if(result){
					goto return_1;
				}

				result = PS_GenChar(CharBuffer2);//生成特征图
				if(result){
					goto return_1;
				}
				add_stage = 3;        //跳转到第三阶段
				break;                

			//第三阶段 比对两枚指纹特征
			case 3:
				result = PS_Match();//比对两枚指纹特征
				if(result)        goto return_1;
				add_stage = 4;                //跳转到第四阶段
				break;

			//第四阶段 特征合并生成模板
			case 4:
				result = PS_RegModel();//特征合并生成模板
				if(result)        goto return_1;
				add_stage = 5;                //跳转到第五阶段                                
				break;
			//第五阶段 检查是否存在
			case 5:
				result = PS_HighSpeedSearch(CharBuffer1,0,99,&ID);
				if(result){	// 不存在
					add_stage = 6;
					break;
				}
				else{
					queue_send_flag = 0;
					return 3;
				}
			//第五阶段 储存模板
			case 6:
				result = PS_StoreChar(CharBuffer2,PageID);//储存模板
				if(result)        goto return_1;
				add_stage = EXIT;
				break;
		}
	}
	queue_send_flag = 0;
	return 0;

	return_1:
	queue_send_flag = 0;
	return 1;

	return_2:
	queue_send_flag = 0;
	return 2;
}

/***************************************************************************
描述: 验证指纹 函数
输入: ID变量地址
返回: 返回状态（0:没完成 1:成功 2:失败）
****************************************************************************/
uint16_t as608_verify_fingerprint(uint16_t* id_val)
{
	uint8_t result;                  	//存放结果
	static uint8_t verify_stage = 1;    //验证的阶段
	uint16_t ID = 0;                 	//存放指纹的ID号

	while(verify_stage != EXIT)		
	//对于验证指纹时不能影响其他任务，不能直接在这循环
	{
		switch(verify_stage)
		{
			//第一阶段 获取指纹图像
			case 1:
				// printf("请放置手指\r\n");                        
				if(as608_detection_finger(100))        return 0x00;        //等待手指按下                        
				result = PS_GetImage();        //获取指纹图像
				if(result)        verify_stage = EXIT;
				verify_stage = 2;
				break;
			
			//第二阶段 生成特征图
			case 2:
				result = PS_GenChar(CharBuffer1);//生成特征图
				if(result)        verify_stage = EXIT;
				verify_stage = 3;
				break;        


			//第三阶段 高速搜索
			case 3:
				result = PS_HighSpeedSearch(CharBuffer1,0,99,&ID);
				if(result)        ID = 0;
				verify_stage = EXIT;
				//此处不加break；

			case EXIT:
				verify_stage = 1;
				if(ID == 0x00){
					printf("验证指纹失败 ID:%d\r\n",ID);
					*id_val = 0x00;
					return 2;
				}
				else{
					printf("验证指纹成功 ID:%d\r\n",ID);
					*id_val = ID;
					return 1;	
				}
		}
	}
	return 0;
}

/***************************************************************************
描述: 删除指纹 函数
返回: 0: 删除指纹成功        1: 没有该指纹			2: 删除失败			3: 未完成
****************************************************************************/
uint8_t as608_delete_fingerprint(void)
{
	uint8_t result;                                        //存放结果
	uint16_t ID;                                        //存放ID号

	// while(as608_verify_fingerprint(&ID)==0){};
	result = as608_verify_fingerprint(&ID);
	if(result == 0){
		return 3;
	}

	if(ID == 0X00){
		printf("删除指纹失败-没有该指纹\r\n");
		return 1;
	}
	//2-针对ID号码进行删除
	result = PS_DeletChar(ID,1);                //删除指纹 ID号 
	if(result){
		printf("删除指纹失败 ID:%d\r\n",ID);
		return 2;
	}
	else{
		printf("删除指纹成功 ID:%d\r\n",ID);
		return 0;
	}		
}

/***************************************************************************
描述: 清空所有指纹 函数
返回: 0: 清空所有指纹成功        1: 清空所有指纹失败
****************************************************************************/
uint8_t as608_empty_all_fingerprint(void)
{
	uint8_t result;                //存放结果
	
	result = PS_Empty();        //删除所有指纹
	switch(result)
	{
			case 0x00:
					printf("清空所有指纹成功\r\n");
					break;
			case 0x01:
					printf("清空所有指纹失败-收包错误\r\n");
					break;
			case 0x11:
					printf("清空所有指纹失败-清空失败\r\n");
					break;        
	}
	if(result) result = 1;
	return result;
}

/***************************************************************************
描述: 查找 指纹个数 函数
返回: 查找到的个数
****************************************************************************/
uint16_t as608_find_fingerprints_num(void)
{
	uint8_t result;
	uint16_t NUM;                //存放指纹个数（模板个数）
	result = PS_ValidTempleteNum(&NUM);
//	if(result)
//			printf("查找指纹个数失败 NUM:%d\r\n",NUM);
//	else
//			printf("查找指纹个数成功 NUM:%d\r\n",NUM);        
	return NUM;
}

uint8_t as608_check_idle_id(uint16_t PageID){
	if( PS_LoadChar(CharBuffer1, PageID) == 0x0c ){	//模板错误
		return 1;
	}
	else{
		return 0;
	}
}

