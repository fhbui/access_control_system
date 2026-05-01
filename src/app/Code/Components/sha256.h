#ifndef __SHA256_H
#define __SHA256_H

#include <stdint.h>
#include <stddef.h>

void sha256_process(const uint8_t* msg, size_t len, uint8_t output[32]);
void sha256_init(void);
void sha256_update(uint8_t* msg, size_t len);
void sha256_final(uint8_t output[32]);

#endif