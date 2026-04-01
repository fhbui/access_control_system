#ifndef __RC522_H
#define __RC522_H

#include <stdint.h>

// 统一状态返回码
typedef enum {
    RC522_OK      = 0x00,
    RC522_NOTAG   = 0x01, // 无卡
    RC522_ERR     = 0x02, // 校验或协议错误
    RC522_TIMEOUT = 0x03  // 响应超时
}rc522_status_t;

#define MAXRLEN               18

/* ------------------- RC522 内部指令集 ------------------- */
#define PCD_IDLE              0x00               // 取消当前命令
#define PCD_AUTHENT           0x0E               // 验证密钥
#define PCD_RECEIVE           0x08               // 接收数据
#define PCD_TRANSMIT          0x04               // 发送数据
#define PCD_TRANSCEIVE        0x0C               // 发送并接收数据
#define PCD_RESETPHASE        0x0F               // 复位
#define PCD_CALCCRC           0x03               // CRC计算

/* ------------------- Mifare_One 卡片命令 ------------------ */
#define PICC_REQIDL           0x26               // 寻天线区内未置暂停状态的卡
#define PICC_REQALL           0x52               // 寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               // 防冲突
#define PICC_AUTHENT1A        0x60               // 验证A密钥
#define PICC_AUTHENT1B        0x61               // 验证B密钥
#define PICC_READ             0x30               // 读块
#define PICC_WRITE            0xA0               // 写块
#define PICC_HALT             0x50               // 休眠

/* ------------------- RC522 寄存器地址 ------------------- */
// PAGE 0
#define CommandReg            0x01    // 启动/停止命令执行
#define ComIEnReg             0x02    // 中断请求使能
#define DivlEnReg             0x03    // 中断请求使能
#define ComIrqReg             0x04    // 中断请求标识
#define DivIrqReg             0x05    // 中断请求标识
#define ErrorReg              0x06    // 错误标识
#define Status1Reg            0x07    // 状态标识
#define Status2Reg            0x08    // 内部状态标识
#define FIFODataReg           0x09    // 64字节FIFO输出入端口
#define FIFOLevelReg          0x0A    // FIFO内字节数
#define WaterLevelReg         0x0B    // FIFO警告阈值
#define ControlReg            0x0C    // 控制寄存器
#define BitFramingReg         0x0D    // 位帧调节
#define CollReg               0x0E    // 冲突检测

// PAGE 1
#define ModeReg               0x11    // 串行通信模式设定
#define TxModeReg             0x12    // 发送速度设定
#define RxModeReg             0x13    // 接收速度设定
#define TxControlReg          0x14    // 控制天线驱动管脚TX1,TX2
#define TxAutoReg             0x15    // 控制天线调制
#define TxSelReg              0x16    // 内部选择 
#define RxSelReg              0x17    // 内部选择
#define RxThresholdReg        0x18    // 位解码阈值
#define DemodReg              0x19    // 解调设定
#define MifareReg             0x1C    // 14443A协议设定

// PAGE 2
#define CRCResultRegM         0x21    // CRC结果高字节
#define CRCResultRegL         0x22    // CRC结果低字节
#define ModeWidthReg          0x24    // 调制脉冲宽度设定
#define RFCfgReg              0x26    // 接收器增益配置
#define GsNReg                0x27    // 驱动管脚电导设定
#define CWGsCfgReg            0x28    // 驱动管脚电导设定
#define ModGsCfgReg           0x29    // 驱动管脚电导设定
#define TModeReg              0x2A    // 定时器内部设定
#define TPrescalerReg         0x2B    // 定时器内部设定
#define TReloadRegH           0x2C    // 定时器重载值(16位)高字节
#define TReloadRegL           0x2D    // 定时器重载值(16位)低字节
#define TCounterValRegH       0x2E    // 定时器当前值高字节
#define TCounterValRegL       0x2F    // 定时器当前值低字节

// PAGE 3
#define VersionReg            0x37    // 版本寄存器          

/* ------------------ 核心暴露接口 ------------------ */

void rc522_init(void);
rc522_status_t rc522_identify_card(uint8_t *uid);
rc522_status_t rc522_select_card(uint8_t *uid);
rc522_status_t rc522_authenticate(uint8_t auth_mode, uint8_t addr, uint8_t *key, uint8_t *uid);
rc522_status_t rc522_read_block(uint8_t addr, uint8_t *data);
rc522_status_t rc522_write_block(uint8_t addr, uint8_t *data);
void rc522_halt(void);

#endif