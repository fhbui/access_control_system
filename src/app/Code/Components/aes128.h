#ifndef __AES128_H
#define __AES128_H

#include <stdint.h>

void aes128_encrypt_16bytes(uint8_t msg[16]);
void aes128_decrypt_16bytes(uint8_t msg[16]);
void aes128_cbc_encrypt(uint8_t* msg_in, uint32_t in_size, uint8_t* msg_out, uint32_t out_size);
void aes128_cbc_decrypt(uint8_t* msg_in, uint32_t in_size, uint8_t* vector_in, uint8_t* msg_out, uint8_t* vector_out);
#endif