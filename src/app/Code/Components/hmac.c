#include "hmac.h"
#include "sha256.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief HMAC procession
 * @param k the key
 * @param k_len the length of key
 * @param msg 
 */
void hmac_process(const uint8_t* k, size_t k_len, const uint8_t* msg, size_t msg_len, uint8_t output[32]){
    uint8_t fixed_k[64] = {0};
    if(k_len > 64){
        sha256_process(k, k_len, fixed_k);
    }
    else{
        memcpy(fixed_k, k, k_len);
    }

    uint8_t* temp = (uint8_t*)calloc(64+64+msg_len, 1);
    uint8_t* temp1 = &temp[64];
    // inner padding
    for(int i=0; i<64; i++){
        temp1[i] = fixed_k[i]^0x36;
    }
    memcpy(&temp1[64], msg, msg_len);
    sha256_process(temp1, 64+msg_len, temp1);

    // outer padding
    for(int i=0; i<64; i++){
        temp[i] = fixed_k[i]^0x5C;
    }
    sha256_process(temp, 64+32, output);

    free(temp);
}

static uint8_t fixed_k[64] = {0};
static uint8_t k_ipad[64];
static uint8_t k_opad[64];

/**
 * @brief Start HMAC process
 */
void hmac_start(const uint8_t* k, size_t k_len){
    if(k_len > 64){
        sha256_init();
        sha256_update((uint8_t*)k, k_len);
        memset(fixed_k, 0, 64);
        sha256_final(fixed_k);      // just output 32 bytes
    }
    else{
        memset(fixed_k, 0, 64);
        memcpy(fixed_k, k, k_len);
    }

    for (int i = 0; i < 64; i++) {
        k_ipad[i] = fixed_k[i] ^ 0x36;
        k_opad[i] = fixed_k[i] ^ 0x5c;
    }

    sha256_init();
    sha256_update(k_ipad, 64);
}

/**
 * @brief Receive message and update the inner SHA256
 */
void hmac_inner_proc(uint8_t* msg, size_t len){
    sha256_update(msg, len);
}

/**
 * @brief Calculate the outter(final) SHA256
 */
void hmac_outter_proc(uint8_t output[32]){
    uint8_t inner_hash[32];
    sha256_final(inner_hash);
    
    sha256_init();
    sha256_update(k_opad, 64);
    sha256_update(inner_hash, 32);
    sha256_final(output);
}
