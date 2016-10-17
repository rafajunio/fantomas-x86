/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#ifndef FANTOMAS_H
#define FANTOMAS_H

#include <stdint.h> 

/**
 * ctr_fantomas16: Fantomas encrypt and decrypt function wit CTR mode.
 * \param io array that will receive the ciphertext or plaintext
 * \param l_io 4-byte containing size of the io
 * \param k 16-byte array containing key
 * \param c 16-byte array containing counter
 */
void ctr_fantomas16(uint8_t *io, uint32_t l_io, uint8_t *k, uint8_t *c);

/***
 * enc_fantomas: encrypt fantomas with both inputs and output aligned in 16. 
 * Input: 'in' the block with plaintext.
 *        'key' key to encrypt
 * Output: 'out' the block with ciphertext.
 */
void enc_fantomas(uint8_t out[16], uint8_t in[16], uint8_t key[16]);

/***
 * dec_fantomas: decrypt fantomas with both inputs and output aligned in 16. 
 * Input: 'in' the block with ciphertext.
 *        'key' key to encrypt
 * Output: 'out' the block with plaintext.
 */
void dec_fantomas(uint8_t out[16], uint8_t in[16], uint8_t key[16]);

#endif /* FANTOMAS_H */

