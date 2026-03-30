#include "rc522.h"
#include "bsp_spi.h"
#include "main.h"
#include <string.h>

#define RC522_CS_LOW()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET)
#define RC522_CS_HIGH()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET)
#define RC522_RST_LOW()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET)
#define RC522_RST_HIGH()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET)

/* ------------------ 私有底层寄存器操作 (内部使用) ------------------ */
/**
 * @brief 向RC522寄存器写入一个字节
 */
static void rc522_write_raw(uint8_t addr, uint8_t val) {
    uint8_t buf[2];
    buf[0] = (addr << 1) & 0x7E;
    buf[1] = val;
    
    RC522_CS_LOW();
    bsp_spi_write(BSP_SPI_2, buf, 2);
    RC522_CS_HIGH();
}

/**
 * @brief 从RC522寄存器读取一个字节
 */
static uint8_t rc522_read_raw(uint8_t addr) {
    uint8_t addr_byte = ((addr << 1) & 0x7E) | 0x80;
    uint8_t val = 0x00;
    
    RC522_CS_LOW();
    bsp_spi_write(BSP_SPI_2, &addr_byte, 1);
    bsp_spi_read(BSP_SPI_2, &val, 1);
    RC522_CS_HIGH();
    
    return val;
}

/**
 * @brief 修改寄存器的位 (SetBit)
 */
void rc522_set_bit(uint8_t addr, uint8_t mask) {
    uint8_t tmp = rc522_read_raw(addr);
    rc522_write_raw(addr, tmp | mask);
}

/**
 * @brief 清除寄存器的位 (ClearBit)
 */
void rc522_clear_bit(uint8_t addr, uint8_t mask) {
    uint8_t tmp = rc522_read_raw(addr);
    rc522_write_raw(addr, tmp & (~mask));
}

/* ------------------- 协议层核心逻辑 ------------------- */
/**
 * @brief 计算CRC16校验
 */
static void rc522_calculate_crc(uint8_t *pInData, uint8_t len, uint8_t *pOutData) {
    uint8_t n;
    rc522_clear_bit(DivIrqReg, 0x04);
    rc522_set_bit(FIFOLevelReg, 0x80);
    
    for (uint8_t i = 0; i < len; i++) {
        rc522_write_raw(FIFODataReg, *(pInData + i));
    }
    rc522_write_raw(CommandReg, PCD_CALCCRC);
    
    uint16_t i = 0xFF; // 等待超时限制
    do {
        n = rc522_read_raw(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    
    pOutData[0] = rc522_read_raw(CRCResultRegL);
    pOutData[1] = rc522_read_raw(CRCResultRegM);
}

/**
 * @brief RC522 与卡片通信的核心函数 (通用命令执行)
 */
rc522_status_t rc522_com_mf522(uint8_t cmd, uint8_t *pInData, uint8_t inLen, uint8_t *pOutData, uint32_t *pOutLenBit) {
    rc522_status_t status = RC522_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits, n;
    
    if (cmd == PCD_AUTHENT) {   // Mifare认证
        irqEn = 0x12; waitFor = 0x10;
    } else if (cmd == PCD_TRANSCEIVE) {     // 接收发送 发送接收
        irqEn = 0x77; waitFor = 0x30;
    }

    rc522_write_raw(ComIEnReg, irqEn | 0x80);
    rc522_clear_bit(ComIrqReg, 0x80);
    rc522_set_bit(FIFOLevelReg, 0x80); 
    rc522_write_raw(CommandReg, PCD_IDLE);

    for (uint8_t i = 0; i < inLen; i++) {
        rc522_write_raw(FIFODataReg, pInData[i]);
    }

    rc522_write_raw(CommandReg, cmd);
    if (cmd == PCD_TRANSCEIVE) {
        rc522_set_bit(BitFramingReg, 0x80); // Start transmission
    }

    uint32_t i = 2000; // 超时长
    do {
        n = rc522_read_raw(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));

    rc522_clear_bit(BitFramingReg, 0x80);

    if (i != 0) {
        if (!(rc522_read_raw(ErrorReg) & 0x1B)) {
            status = RC522_OK;
            if (n & irqEn & 0x01) status = RC522_NOTAG;
            
            if (cmd == PCD_TRANSCEIVE) {
                n = rc522_read_raw(FIFOLevelReg);
                lastBits = rc522_read_raw(ControlReg) & 0x07;
                if (lastBits) *pOutLenBit = (n - 1) * 8 + lastBits;
                else *pOutLenBit = n * 8;
                
                if (n == 0) n = 1;
                if (n > MAXRLEN) n = MAXRLEN;
                for (uint8_t j = 0; j < n; j++) {
                    pOutData[j] = rc522_read_raw(FIFODataReg);
                }
            }
        }
    }
    return status;
}


/* ------------------ 业务逻辑导出接口 ------------------ */

/**
 * @brief 模块初始化
 */
void rc522_init(void){
    // 硬件 Reset
    RC522_CS_LOW();
    HAL_Delay(10);
    RC522_CS_HIGH();
    HAL_Delay(10);

    // 初始寄存器配置
    rc522_write_raw(CommandReg, 0x0F);    // 复位
    rc522_write_raw(ModeReg, 0x3D);       // 定义发送和接收模式
    rc522_write_raw(TReloadRegL, 30);     // 16位定时器重装值
    rc522_write_raw(TReloadRegH, 0);
    rc522_write_raw(TModeReg, 0x8D);      // 定时器内部设置
    rc522_write_raw(TPrescalerReg, 0x3E); // 定时器频率
    rc522_write_raw(TxAutoReg, 0x40);

    // 强制开启天线
    uint8_t tmp = rc522_read_raw(TxControlReg);
    if (!(tmp & 0x03)) {
        rc522_set_bit(TxControlReg, 0x03);
    }
}

/**
 * @brief 寻卡并成功获取UID的原子组合操作
 * @param uid 存储获取到的4字节UID
 * @return rc522_status_t 
 */
rc522_status_t rc522_identify_card(uint8_t *uid){
    uint8_t buf[MAXRLEN];
    uint32_t len;
    rc522_status_t status;

    rc522_clear_bit(Status2Reg, 0x08);
    rc522_write_raw(BitFramingReg, 0x07);

    // 1. Request
    buf[0] = PICC_REQIDL;
    status = rc522_com_mf522(PCD_TRANSCEIVE, buf, 1, buf, &len);
    if (status != RC522_OK) return RC522_NOTAG;

    // 2. Anticoll (获取UID)
    rc522_write_raw(BitFramingReg, 0x00);
    buf[0] = PICC_ANTICOLL1;
    buf[1] = 0x20;
    status = rc522_com_mf522(PCD_TRANSCEIVE, buf, 2, buf, &len);
    
    if (status == RC522_OK) {
		uint8_t bcc = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
		if (bcc == buf[4]) {
			memcpy(uid, buf, 4);
		} else {
			return RC522_ERR; // 校验失败，说明是无效读取
		}
    }
    
    return status;
}

/**
 * @brief 验证扇区密码
 * @param auth_mode 验证模式：PICC_AUTHENT1A (0x60) 或 PICC_AUTHENT1B (0x61)
 * @param addr 块地址
 * @param key  6字节密钥
 * @param uid  4字节卡号
 */
rc522_status_t rc522_authenticate(uint8_t auth_mode, uint8_t addr, uint8_t *key, uint8_t *uid){
    uint8_t buf[MAXRLEN];
    uint32_t len;
    
    buf[0] = auth_mode;
    buf[1] = addr;
    memcpy(&buf[2], key, 6);
    memcpy(&buf[8], uid, 4);

    rc522_status_t status = rc522_com_mf522(PCD_AUTHENT, buf, 12, buf, &len);
    
    if ((status != RC522_OK) || (!(rc522_read_raw(Status2Reg) & 0x08))) {
        return RC522_ERR;
    }
    return RC522_OK;
}

/**
 * @brief 读/写数据块 (16字节)
 */
rc522_status_t rc522_read_block(uint8_t addr, uint8_t *data){
    uint8_t buf[MAXRLEN];
    uint32_t len;
    
    buf[0] = PICC_READ;
    buf[1] = addr;
    rc522_calculate_crc(buf, 2, &buf[2]);
    
    rc522_status_t status = rc522_com_mf522(PCD_TRANSCEIVE, buf, 4, buf, &len);
    if (status == RC522_OK && len == 0x90) {
        memcpy(data, buf, 16);
    } else {
        status = RC522_ERR;
    }
    return status;
}

rc522_status_t rc522_write_block(uint8_t addr, uint8_t *data){
    uint8_t buf[MAXRLEN];
    uint32_t len;
    
    buf[0] = PICC_WRITE;
    buf[1] = addr;
    rc522_calculate_crc(buf, 2, &buf[2]);
    
    rc522_status_t status = rc522_com_mf522(PCD_TRANSCEIVE, buf, 4, buf, &len);
    if (status != RC522_OK || (buf[0] & 0x0F) != 0x0A) return RC522_ERR;
    
    memcpy(buf, data, 16);
    rc522_calculate_crc(buf, 16, &buf[16]);
    status = rc522_com_mf522(PCD_TRANSCEIVE, buf, 18, buf, &len);
    
    if (status != RC522_OK || (buf[0] & 0x0F) != 0x0A) return RC522_ERR;
    
    return RC522_OK;
}

/**
 * @brief 卡休眠
 */
void rc522_halt(void){
    uint8_t buf[4];
    uint32_t len;
    buf[0] = PICC_HALT;
    buf[1] = 0;
    rc522_calculate_crc(buf, 2, &buf[2]);
    rc522_com_mf522(PCD_TRANSCEIVE, buf, 4, buf, &len);
}
