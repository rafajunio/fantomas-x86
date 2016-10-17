/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#ifndef MODE_H
#define MODE_H

#include <stdint.h> 

/***
 * ctr128_raw: encrypt and decrypt function with CTR mode. 
 * Input: 'io' array  with the ciphertext or plaintext
 *        'l_io' 4-byte containing size of the io
 *        'enc' the encrypt function of the block cipher
 *        'k' 16-byte array containing key
 *        'c' 16-byte array containing counter
 * Output: 'io' array  with the plaintext or ciphertext
 */
void ctr128_raw(uint8_t *io, uint32_t l_io, void (*enc) (uint8_t *, uint8_t *, uint8_t *), uint8_t *k, uint8_t *c) ;


/***
 * enc_cbc128_raw: encrypt function with CBC mode. 
 * Input: 'io' array  with the plaintext
 *        'l_io' 4-byte containing size of the io
 *        'enc' the encrypt function of the block cipher
 *        'k' 16-byte array containing key
 *        'iv' 16-byte array containing initialization vector
 * Output: 'io' array  with the ciphertext
 */
 void enc_cbc128_raw(uint8_t *io, uint32_t l_io, void (*enc) (uint8_t *, uint8_t *, uint8_t *), uint8_t *k, uint8_t *iv) ;


/***
 * dec_cbc128_raw: decrypt function with CBC mode. 
 * Input: 'io' array  with the ciphertext
 *        'l_io' 4-byte containing size of the io
 *        'dec' the decrypt function of the block cipher
 *        'k' 16-byte array containing key
 *        'iv' 16-byte array containing initialization vector
 * Output: 'io' array  with the plaintext
 */
void dec_cbc128_raw(uint8_t *io, uint32_t l_io, void (*dec) (uint8_t *, uint8_t *, uint8_t *), uint8_t *k, uint8_t *iv) ;

#endif /* MODE_H */


