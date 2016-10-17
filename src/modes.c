/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#include "modes.h"


/***
 * ctr128_raw: encrypt and decrypt function with CTR mode. 
 * Input: 'io' array  with the ciphertext or plaintext
 *        'l_io' 4-byte containing size of the io
 *        'enc' the encrypt function of the block cipher
 *        'k' 16-byte array containing key
 *        'c' 16-byte array containing counter
 * Output: 'io' array  with the plaintext or ciphertext
 */
void ctr128_raw(uint8_t *io, uint32_t l_io, void (*enc) (uint8_t *, uint8_t *, uint8_t *), uint8_t *k, uint8_t *c) {
    uint8_t out[16], new_c[16], carry;
    uint32_t blocks = l_io >> 4;  //number of the complet blocks
    uint32_t *new_c_32 = (uint32_t *)new_c, *out_32 = (uint32_t *)out, *io_32 = (uint32_t *)io, *c_32 = (uint32_t *)c;
    register uint32_t i;
    register int32_t  j;
        
    for(i=0; i<4; i++) {
        new_c_32[i] = c_32[i];
    }
    
    //complete blocks
    for(i=0; i<blocks; i++) {
        enc(out, new_c, k);
        for (j=15, carry=1; carry && j>=0 ; j--) {
            carry = !++new_c[j];
        }
        for(j=0; j<4; j++) { //copy output
            io_32[i*4 + j] = out_32[j] ^ io_32[i*4 + j];
        }
    }
}


/***
 * enc_cbc128_raw: encrypt function with CBC mode. 
 * Input: 'io' array  with the plaintext
 *        'l_io' 4-byte containing size of the io
 *        'enc' the encrypt function of the block cipher
 *        'k' 16-byte array containing key
 *        'iv' 16-byte array containing initialization vector
 * Output: 'io' array  with the ciphertext
 */
void enc_cbc128_raw(uint8_t *io, uint32_t l_io, void (*enc) (uint8_t *, uint8_t *, uint8_t *), uint8_t *k, uint8_t *iv) {
    uint8_t out[16], in[16], new_iv[16];
    uint32_t blocks = l_io >> 4;  //number of the complet blocks
    uint32_t *new_iv_32 = (uint32_t *) new_iv, *out_32 = (uint32_t *) out, *in_32 = (uint32_t *) in, *iv_32 = (uint32_t *) iv, *io_32 = (uint32_t *)io; 
    register uint32_t i,j;

    for(i=0; i<4; i++){
      new_iv_32[i] = iv_32[i];
    }

    //complete blocks
    for(i=0; i<blocks; i++) {
        for(j=0; j<4; j++) {
            in_32[j] = io_32[i*4 + j] ^ new_iv_32[j];
        }
        enc(out, in, k);
        for(j=0; j<4; j++) { //copy output
            io_32[i*4 + j] = new_iv_32[j] = out_32[j];
        }
    }
}


/***
 * dec_cbc128_raw: decrypt function with CBC mode. 
 * Input: 'io' array  with the ciphertext
 *        'l_io' 4-byte containing size of the io
 *        'dec' the decrypt function of the block cipher
 *        'k' 16-byte array containing key
 *        'iv' 16-byte array containing initialization vector
 * Output: 'io' array  with the plaintext
 */
void dec_cbc128_raw(uint8_t *io, uint32_t l_io, void (*dec) (uint8_t *, uint8_t *, uint8_t *), uint8_t *k, uint8_t *iv) {
    uint8_t out[16], in[16], new_iv[16];
    uint32_t blocks = l_io >> 4; //number of the complet blocks
    uint32_t *new_iv_32 = (uint32_t *) new_iv, *out_32 = (uint32_t *) out, *in_32 = (uint32_t *) in, *iv_32 = (uint32_t *) iv, *io_32 = (uint32_t *)io; 
    register uint32_t i,j;

    for(i=0; i<4; i++){
      new_iv_32[i] = iv_32[i];
    }

    //complete blocks
    for(i=0; i<blocks; i++) {
        for(j=0; j<4; j++) { //copy block
            in_32[j] = io_32[i*4 + j];
        }
        dec(out, in, k);
        for(j=0; j<4; j++) { //copy output
            io_32[i*4 + j] = out_32[j] ^ new_iv_32[j];
            new_iv_32[j] = in_32[j]; 
        }
    }
}


