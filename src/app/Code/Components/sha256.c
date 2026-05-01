#include "sha256.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// data type transform. SHA256 require big-endian
#define GET_UINT32_BE(n, b, i)      n = ((uint32_t)b[i]<<24) | ((uint32_t)b[i+1]<<16) | ((uint32_t)b[i+2]<<8) | ((uint32_t)b[i+3])
#define PUT_UINT32_BE(n, b, i)      {   \
    b[i] = (uint8_t)(n>>24);    \
    b[i+1] = (uint8_t)(n>>16);  \
    b[i+2] = (uint8_t)(n>>8);   \
    b[i+3] = (uint8_t)(n);      \
}

// bit function
#define ROTRIGHT(word, n)    ((word>>n)|(word<<(32-n)))      // for 32bits

#define CH(x, y, z)     ((x&y)^(~x&z))
#define MAJ(x, y, z)    ((x&y)^(x&z)^(y&z))
#define EP0(x)          (ROTRIGHT(x,2)^ROTRIGHT(x,13)^ROTRIGHT(x,22))
#define EP1(x)          (ROTRIGHT(x,6)^ROTRIGHT(x,11)^ROTRIGHT(x,25))
#define SIG0(x)         (ROTRIGHT(x,7)^ROTRIGHT(x,18)^(x>>3))
#define SIG1(x)         (ROTRIGHT(x,17)^ROTRIGHT(x,19)^(x>>10))

// fixed datum used in transform
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};



/**
 * @brief main transform function of SHA256
 * @param H hash value input
 * @param m input message datum, need 64 bytes (as 512 bits)
 */
static void sha256_transform(uint32_t H[8], const uint8_t m[64]){
    uint32_t a,b,c,d,e,f,g,h;
    uint32_t t1, t2, w[64];

    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];

    // calculate w
    for(int i=0; i<16; i++){
        GET_UINT32_BE(w[i], m, i*4);
    }
    for(int i=16; i<64; i++){
        w[i] = SIG1(w[i-2])+w[i-7]+SIG0(w[i-15])+w[i-16];
    }

    // update value
    for(int i=0; i<64; i++){
        t1 = h+EP1(e)+CH(e,f,g)+k[i]+w[i];
        t2 = EP0(a)+MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
    H[5] += f;
    H[6] += g;
    H[7] += h;
}

/**
 * @brief procession of SHA256
 * @param msg input message
 * @param len the length of input message (/byte)
 * @param output output hash result
 */
void sha256_process(const uint8_t* msg, size_t len, uint8_t output[32]){
    uint32_t hash[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    // preprocess
    size_t padded_len = len+1+8;
    while(padded_len%64 != 0)   padded_len++;

    uint8_t* padded_msg = (uint8_t*)calloc(padded_len, 1);  // calloc will set the members 0
    memcpy(padded_msg, msg, len);
    padded_msg[len] = 0x80;     // 1000 0000

    // add len value (64bits)
    uint64_t msg_bits = (uint64_t)len * 8;
    for (int i = 0; i < 8; i++) {
        padded_msg[padded_len - 1 - i] = (uint8_t)(msg_bits >> (i * 8));  
    }

    // for each 512 bits(64 bytes)
    for(size_t i=0; i<padded_len; i+=64){
        sha256_transform(hash, &padded_msg[i]);
    }

    for(int i=0; i<8; i++){
        PUT_UINT32_BE(hash[i], output, i*4);
    }
    free(padded_msg);
}

typedef struct{
    uint32_t hash[8];
    size_t count;
    uint8_t buffer[64];
    size_t data_len;
}sha256_ctx_t;

static sha256_ctx_t sha256_ctx;

void sha256_init(void){
    sha256_ctx.hash[0] = 0x6a09e667;
    sha256_ctx.hash[1] = 0xbb67ae85;
    sha256_ctx.hash[2] = 0x3c6ef372;
    sha256_ctx.hash[3] = 0xa54ff53a;
    sha256_ctx.hash[4] = 0x510e527f;
    sha256_ctx.hash[5] = 0x9b05688c;
    sha256_ctx.hash[6] = 0x1f83d9ab;
    sha256_ctx.hash[7] = 0x5be0cd19;

    sha256_ctx.count = 0;
    sha256_ctx.data_len = 0;
}

/**
 * @brief SHA256 update hash state
 */
void sha256_update(uint8_t* msg, size_t len){
    // No Padding
    size_t i = 0;
    sha256_ctx.count += (uint64_t)len*8;

    if(sha256_ctx.data_len > 0){
        i = 64 - sha256_ctx.data_len;
        if(len >= i){
            memcpy(&sha256_ctx.buffer[sha256_ctx.data_len], msg, i);
            sha256_transform(sha256_ctx.hash, sha256_ctx.buffer);
            sha256_ctx.data_len = 0;
        }
        else{   // The length of msg is not enough to fill ctx.buffer
            memcpy(&sha256_ctx.buffer[sha256_ctx.data_len], msg, len);
            sha256_ctx.data_len += len;
            return ;
        }
    }
    
    while(i+64 <= len){
        sha256_transform(sha256_ctx.hash, &msg[i]);
        i += 64;
    }
    if(len > i){
        memcpy(&sha256_ctx.buffer[sha256_ctx.data_len], &msg[i], len-i);
        sha256_ctx.data_len += len-i;
    }
}

/**
 * @brief Final process of SHA256
 */
void sha256_final(uint8_t output[32]){
    // Padding
    size_t len = sha256_ctx.data_len;
    size_t padded_len = len+1+8;
    while(padded_len%64 != 0)   padded_len++;

    uint8_t* padded_msg = (uint8_t*)calloc(padded_len, 1);  // calloc will set the members 0
    memcpy(padded_msg, sha256_ctx.buffer, len);
    padded_msg[len] = 0x80;     // 1000 0000

    // add len value (64bits)
    uint64_t msg_bits = (uint64_t)(sha256_ctx.count);
    for (int i = 0; i < 8; i++) {
        padded_msg[padded_len - 1 - i] = (uint8_t)(msg_bits >> (i * 8));  
    }

    // for each 512 bits(64 bytes)
    for(size_t i=0; i<padded_len; i+=64){
        sha256_transform(sha256_ctx.hash, &padded_msg[i]);
    }

    for(int i=0; i<8; i++){
        PUT_UINT32_BE(sha256_ctx.hash[i], output, i*4);
    }
    free(padded_msg);
}