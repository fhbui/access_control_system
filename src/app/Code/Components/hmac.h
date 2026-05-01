#ifndef __HMAC_H
#define __HMAC_H

#include <stdint.h>
#include <stddef.h>

void hmac_start(const uint8_t* k, size_t k_len);
void hmac_inner_proc(uint8_t* msg, size_t len);
void hmac_outter_proc(uint8_t output[32]);
#endif