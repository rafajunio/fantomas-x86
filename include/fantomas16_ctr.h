/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#ifndef FANTOMAS16_CTR_H
#define FANTOMAS16_CTR_H

#include <stdint.h> 

/**
 * ctr_fantomas16: Fantomas encrypt and decrypt function wit CTR mode.
 * \param io array that will receive the ciphertext or plaintext
 * \param l_io 4-byte containing size of the io
 * \param k 16-byte array containing key
 * \param c 16-byte array containing counter
 */
void ctr_fantomas16(uint8_t *io, uint32_t l_io, uint8_t *k, uint8_t *c);


/**
 * enc_fantomas: Fantomas encrypt function.
 * \param out 16-byte array that will receive the ciphertext
 * \param in 16-byte array containing plaintext
 * \param key 16-byte array containing key
 */

void enc_fantomas(uint8_t out[16], uint8_t in[16], uint8_t key[16]);

/**
 * dec_fantomas: Fantomas decrypt function.
 * \param out 16-byte array that will receive the plaintext
 * \param in 16-byte array containing ciphertext
 * \param key 16-byte array containing key
 */

void dec_fantomas(uint8_t out[16], uint8_t in[16], uint8_t key[16]);


#endif /* FANTOMAS16_CTR_H */
