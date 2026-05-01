 #include "aes128.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
static uint8_t round_keys[176] = {0};       // 11*16
uint8_t vector_IV[16] = {0};

// S box transfer: value to be converted (8 bits) decide index
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t rcon[11] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

/**
 * @brief 有限域 GF(2^8) 上的乘 2 运算
 * @param x 输入的字节
 * @return uint8_t 运算结果
 */
static uint8_t gmul_2(uint8_t x) {
    return (x << 1) ^ (((x >> 7) & 1) ? 0x1b : 0);
}

/**
 * @brief AES-128 密钥扩展算法
 */
static void key_expansion(void){
    memcpy(round_keys, key, 16);    // devide key into w[0]~w[4] directly

    for(int i=16; i<176; i+=4){      // for each W[], 4 indexs between two W[]
        uint8_t last_w[4];
        memcpy(last_w, &round_keys[i-4], 4); // copy last W[]

        if(i%16==0){
            // g function
            uint8_t temp = last_w[0];
            last_w[0] = sbox[last_w[1]] ^ rcon[i/16];
            last_w[1] = sbox[last_w[2]];
            last_w[2] = sbox[last_w[3]];
            last_w[3] = sbox[temp];
        }

        // calculate new W[]
        for(int j=0; j<4; j++){
            round_keys[i+j] = round_keys[i-16+j]^last_w[j];
        }
    }
}

/**
 * @brief 轮密钥加 (AddRoundKey)
 * @param state 当前状态矩阵（16 字节）
 * @param roundkeys 当前轮次的密钥指针
 */
static void add_round_key(uint8_t state[16], uint8_t roundkeys[16]){
    for(int i=0; i<16; i++){
        state[i] ^= roundkeys[i];
    }
}

//=================== encrypt =========================
#if 0

/**
 * @brief 字节代换 (SubBytes)
 * @details 使用 AES S 盒 (Substitution Box) 对状态矩阵中的每个字节进行非线性映射。
 * @param state 当前状态矩阵
 */
static void sub_bytes(uint8_t state[16]){
    for(int i=0; i<16; i++){
        state[i] = sbox[state[i]];
    }
}

/**
 * @brief 行移位 (ShiftRows)
 * @details 对状态矩阵的每一行执行不同偏移量的循环左移。
 * @param state 当前状态矩阵
 */
static void shift_rows(uint8_t state[16]){
    uint8_t temp[16];
    memcpy(temp, state, 16);

    // second row
    state[1] = temp[5]; state[5] = temp[9]; state[9] = temp[13]; state[13] = temp[1];
    // third row
    state[2] = temp[10]; state[6] = temp[14]; state[10] = temp[2]; state[14] = temp[6];
    // fourth row
    state[3] = temp[15]; state[7] = temp[3]; state[11] = temp[7]; state[15] = temp[11];
}

/**
 * @brief 列混淆 (MixColumns)
 * @details 在有限域 GF(2^8) 上通过矩阵乘法混淆状态矩阵的列。
 * @param state 当前状态矩阵
 */
static void mix_colums(uint8_t state[16]){
    // expand matrix operations into expression
    for (int i = 0; i < 4; i++) {
        uint8_t a = state[i*4], b = state[i*4+1], c = state[i*4+2], d = state[i*4+3];
        state[i*4]   = gmul_2(a) ^ (gmul_2(b) ^ b) ^ c ^ d;
        state[i*4+1] = a ^ gmul_2(b) ^ (gmul_2(c) ^ c) ^ d;
        state[i*4+2] = a ^ b ^ gmul_2(c) ^ (gmul_2(d) ^ d);
        state[i*4+3] = (gmul_2(a) ^ a) ^ b ^ c ^ gmul_2(d);
    }
}

/**
 * @brief 单个 16 字节块的 AES-128 加密
 * @param msg 待加密的 16 字节明文块，加密结果将直接存入此空间
 */
void aes128_encrypt_16bytes(uint8_t msg[16]){

    key_expansion();
    add_round_key(msg, &round_keys[0]);

    for(int i=1; i<10; i++){
        sub_bytes(msg);
        shift_rows(msg);
        mix_colums(msg);
        add_round_key(msg, &round_keys[16*i]);
    }

    sub_bytes(msg);
    shift_rows(msg);
    add_round_key(msg, &round_keys[160]);
}

/**
 * @brief AES-128 CBC 模式加密（支持 PKCS#7 填充）
 * @details 使用密码分组链接 (CBC) 模式对数据进行加密。
 *          逻辑：当前块明文先与前一密文块进行 XOR，再进行 AES 加密。
 *          包含 PKCS#7 填充处理：若数据量不足或恰好为 16 倍数，会自动添加填充块。
 * @param msg_in   输入明文指针
 * @param in_size  明文长度（字节）
 * @param msg_out  输出密文缓冲区指针
 * @param out_size 输出缓冲区容量（应大于输入长度并按 16 字节对齐）
 */
void aes128_cbc_encrypt(uint8_t* msg_in, uint32_t in_size, uint8_t* msg_out, uint32_t out_size){
    if(out_size <= in_size || out_size%16!=0){
        return ;
    }

    uint8_t pre_block[16];      // it store IV at first
    uint8_t cur_block[16];

    memcpy(pre_block, vector_IV, 16);
    for(int i=0; i<in_size; i+=16){
        int remain = in_size - i;
        if(remain < 16){
            memset(cur_block, (uint8_t)(16-remain), 16);
            memcpy(cur_block, &msg_in[i], remain);
        }
        else{
            memcpy(cur_block, &msg_in[i], 16);
        }

        for(int j=0; j<16; j++){
            cur_block[j] ^= pre_block[j];
        }

        aes128_encrypt_16bytes(cur_block);
        memcpy(pre_block, cur_block, 16);

        if(remain < 16){
            memcpy(&msg_out[i], cur_block, remain);
        }
        else{
            memcpy(&msg_out[i], cur_block, 16);
        }

        // add a new msg because of PKCS#7
        if(remain == 16){
            memset(cur_block, (uint8_t)0x10, 16);
            for(int j=0; j<16; j++){
                cur_block[j] ^= pre_block[j];
            }
            aes128_encrypt_16bytes(cur_block);
            memcpy(&msg_out[i+16], cur_block, 16);  // size of msg_out should larger than msg_in, and be 16*n
        }
    }
}
#endif
//=================== decrypt =========================

// reverse S box
static const uint8_t rsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static inline uint8_t gmul_4(uint8_t x) { return gmul_2(gmul_2(x)); }
static inline uint8_t gmul_8(uint8_t x) { return gmul_2(gmul_4(x)); }

static inline uint8_t gmul_9(uint8_t x)  { return gmul_8(x) ^ x; }
static inline uint8_t gmul_11(uint8_t x) { return gmul_8(x) ^ gmul_2(x) ^ x; }
static inline uint8_t gmul_13(uint8_t x) { return gmul_8(x) ^ gmul_4(x) ^ x; }
static inline uint8_t gmul_14(uint8_t x) { return gmul_8(x) ^ gmul_4(x) ^ gmul_2(x); }

/**
 * @brief 逆列混淆 (InvMixColumns)
 * @details 加密列混淆的逆运算，使用不同的域乘法系数。
 * @param state 当前状态矩阵
 */
static void inv_mix_columns(uint8_t state[16]){
    for (int i = 0; i < 4; i++) {
        uint8_t a = state[i*4], b = state[i*4+1], c = state[i*4+2], d = state[i*4+3];
        
        // prepare the value to reduce calculated amount
        uint8_t a2 = gmul_2(a), a4 = gmul_2(a2), a8 = gmul_2(a4);
        uint8_t b2 = gmul_2(b), b4 = gmul_2(b2), b8 = gmul_2(b4);
        uint8_t c2 = gmul_2(c), c4 = gmul_2(c2), c8 = gmul_2(c4);
        uint8_t d2 = gmul_2(d), d4 = gmul_2(d2), d8 = gmul_2(d4);

        state[i*4]   = (a8 ^ a4 ^ a2) ^ (b8 ^ b2 ^ b) ^ (c8 ^ c4 ^ c) ^ (d8 ^ d);
        state[i*4+1] = (a8 ^ a) ^ (b8 ^ b4 ^ b2) ^ (c8 ^ c2 ^ c) ^ (d8 ^ d4 ^ d);
        state[i*4+2] = (a8 ^ a4 ^ a) ^ (b8 ^ b) ^ (c8 ^ c4 ^ c2) ^ (d8 ^ d2 ^ d);
        state[i*4+3] = (a8 ^ a2 ^ a) ^ (b8 ^ b4 ^ b) ^ (c8 ^ c) ^ (d8 ^ d4 ^ d2);
    }
}

/**
 * @brief 逆行移位 (InvShiftRows)
 * @details 加密行移位的逆运算，各行执行循环右移。
 * @param state 当前状态矩阵
 */
static void inv_shift_rows(uint8_t state[16]){
    uint8_t temp[16];
    memcpy(temp, state, 16);

    // second row
    state[1] = temp[13]; state[5] = temp[1]; state[9] = temp[5]; state[13] = temp[9];
    // third row
    state[2] = temp[10]; state[6] = temp[14]; state[10] = temp[2]; state[14] = temp[6];
    // fourth row
    state[3] = temp[7]; state[7] = temp[11]; state[11] = temp[15]; state[15] = temp[3];
}

/**
 * @brief 逆字节代换 (InvSubBytes)
 * @details 使用逆 S 盒对状态矩阵进行映射。
 * @param state 当前状态矩阵
 */
static void inv_sub_bytes(uint8_t state[16]){
    for(int i=0; i<16; i++){
        state[i] = rsbox[state[i]];
    }
}

/**
 * @brief 单个 16 字节块的 AES-128 解密
 * @param msg 待解密的 16 字节密文块，解密结果将直接存入此空间
 */
void aes128_decrypt_16bytes(uint8_t msg[16]){
    key_expansion();
    add_round_key(msg, &round_keys[160]);
    inv_shift_rows(msg);
    inv_sub_bytes(msg);
    
    for(int i=9; i>=1; i--){
        add_round_key(msg, &round_keys[16*i]);
        inv_mix_columns(msg);
        inv_shift_rows(msg);
        inv_sub_bytes(msg);
    }
    add_round_key(msg, &round_keys[0]);
}

/**
 * @brief AES-128 CBC 模式解密
 * @details 逻辑：先对密文块执行 AES 解密，再将结果与前一密文块执行 XOR。
 * @param msg_in     输入密文指针
 * @param in_size    密文长度（需 16 字节对齐）
 * @param vector_in  输入初始向量 IV (16字节)
 * @param msg_out    输出明文缓冲区指针
 * @param vector_out 可选：输出最后一个密文块，可作为下一链条的 IV
 */
void aes128_cbc_decrypt(uint8_t* msg_in, uint32_t in_size, uint8_t* vector_in, uint8_t* msg_out, uint8_t* vector_out){
    uint8_t pre_block[16];
    uint8_t cur_block[16];
	memcpy(pre_block, vector_in, 16);

    for(int i=0; i<in_size; i+=16){
        memcpy(cur_block, &msg_in[i], 16);
        aes128_decrypt_16bytes(cur_block);
        for(int j=0; j<16; j++){
            cur_block[j] ^= pre_block[j];
        }
        memcpy(pre_block, &msg_in[i], 16);
        memcpy(&msg_out[i], cur_block, 16);
    }
    if(vector_out != NULL){
        memcpy(vector_out, pre_block, 16);
    }
}
