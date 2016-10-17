/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#include "fantomas.h"
#include <tmmintrin.h> //SSSE3

typedef    uint8_t    v16qu  __attribute__ ((vector_size (16)));
typedef    int16_t     v8hu  __attribute__ ((vector_size (16)));
typedef    int16_t     v8hi  __attribute__ ((vector_size (16)));

#define V(x,y) (v16qu) {x, y, x, y, x, y, x, y, x, y, x, y, x, y, x, y}


/***
 * SLayer: Performs substitution in one block using Misty 3/5 bits. 
 * The 128-bits register contain one block.
 * Input: 'X'_i in {0, 1, ..., 7} where X_i have 16 bits.
 * Output: 'X'_i with Misty 3/5 bits applied.
 */
#define SLayer(X) {             \
    /* S5 */                    \
    X[2] ^= X[0] & X[1];        \
    X[1] ^= X[2];               \
    X[3] ^= X[0] & X[4];        \
    X[2] ^= X[3];               \
    X[0] ^= X[1] & X[3];        \
    X[4] ^= X[1];               \
    X[1] ^= X[2] & X[4];        \
    X[1] ^= X[0];               \
                                \
    /* Extend-Xor */            \
    X[0] ^= X[5];               \
    X[1] ^= X[6];               \
    X[2] ^= X[7];               \
                                \
    /* Key */                   \
    X[3] = ~X[3];               \
    X[4] = ~X[4];               \
                                \
    /* S3: 3-bit Keccak S-box */\
    __typeof(X[0]) t0 = X[5],   \
                   t1 = X[6],   \
                   t2 = X[7];   \
    X[5] ^= (~t1) & t2;         \
    X[6] ^= (~t2) & t0;         \
    X[7] ^= (~t0) & t1;         \
                                \
    /* Truncate-Xor */          \
    X[5] ^= X[0];               \
    X[6] ^= X[1];               \
    X[7] ^= X[2];               \
                                \
    /* S5 */                    \
    X[2] ^= X[0] & X[1];        \
    X[1] ^= X[2];               \
    X[3] ^= X[0] & X[4];        \
    X[2] ^= X[3];               \
    X[0] ^= X[1] & X[3];        \
    X[4] ^= X[1];               \
    X[1] ^= X[2] & X[4];        \
    X[1] ^= X[0];               \
}


/***
 * SLayerInv: Performs inverse substitution in one block using Misty 3/5 bits. 
 * The 128-bits register contain one block.
 * Input: 'X'_i in {0, 1, ..., 7} where X_i have 16 bits.
 * Output: 'X'_i with Misty 3/5 bits applied.
 */
#define SLayerInv(X) {          \
    /* S5 reverse */            \
    X[1] ^= X[0];               \
    X[1] ^= X[2] & X[4];        \
    X[4] ^= X[1];               \
    X[0] ^= X[1] & X[3];        \
    X[2] ^= X[3];               \
    X[3] ^= X[0] & X[4];        \
    X[1] ^= X[2];               \
    X[2] ^= X[0] & X[1];        \
                                \
    /* Truncate-Xor */          \
    X[5] ^= X[0];               \
    X[6] ^= X[1];               \
    X[7] ^= X[2];               \
                                \
    /* S3: 3-bit Keccak S-box */\
    __typeof(X[0]) t0 = X[5],   \
                   t1 = X[6],   \
                   t2 = X[7];   \
    X[5] ^= (~t1) & t2;         \
    X[6] ^= (~t2) & t0;         \
    X[7] ^= (~t0) & t1;         \
                                \
    /* Key */                   \
    X[3] = ~X[3];               \
    X[4] = ~X[4];               \
                                \
    /* Extend-Xor */            \
    X[0] ^= X[5];               \
    X[1] ^= X[6];               \
    X[2] ^= X[7];               \
                                \
    /* S5 Reverse */            \
    X[1] ^= X[0];               \
    X[1] ^= X[2] & X[4];        \
    X[4] ^= X[1];               \
    X[0] ^= X[1] & X[3];        \
    X[2] ^= X[3];               \
    X[3] ^= X[0] & X[4];        \
    X[1] ^= X[2];               \
    X[2] ^= X[0] & X[1];        \
}


/***
 * LLayer: Performs linear transformation in one block using the L-box. 
 * The 128-bits register contain one block.
 * Input: 'X'_i in {0, 1, ..., 7} where X_i have 16-bits.
 * Output: 'return value' with L-box applied.
 */
static v8hu LLayer(v8hu X) {
    static const v16qu tables[8] =
    {
        {0x00, 0xFF, 0x90, 0x6F, 0x37, 0xC8, 0xA7, 0x58, 0x48, 0xB7, 0xD8, 0x27, 0x7F, 0x80, 0xEF, 0x10},
        {0x00, 0xBF, 0x6E, 0xD1, 0x41, 0xFE, 0x2F, 0x90, 0xD5, 0x6A, 0xBB, 0x04, 0x94, 0x2B, 0xFA, 0x45},
        {0x00, 0x96, 0x83, 0x15, 0xB1, 0x27, 0x32, 0xA4, 0x6C, 0xFA, 0xEF, 0x79, 0xDD, 0x4B, 0x5E, 0xC8},
        {0x00, 0x52, 0xD1, 0x83, 0xC2, 0x90, 0x13, 0x41, 0x49, 0x1B, 0x98, 0xCA, 0x8B, 0xD9, 0x5A, 0x08},
        {0x00, 0xC2, 0x24, 0xE6, 0x28, 0xEA, 0x0C, 0xCE, 0x56, 0x94, 0x72, 0xB0, 0x7E, 0xBC, 0x5A, 0x98},
        {0x00, 0x74, 0xD3, 0xA7, 0xCE, 0xBA, 0x1D, 0x69, 0x68, 0x1C, 0xBB, 0xCF, 0xA6, 0xD2, 0x75, 0x01},
        {0x00, 0x89, 0x39, 0xB0, 0x44, 0xCD, 0x7D, 0xF4, 0x4B, 0xC2, 0x72, 0xFB, 0x0F, 0x86, 0x36, 0xBF},
        {0x00, 0xE4, 0x68, 0x8C, 0x5E, 0xBA, 0x36, 0xD2, 0x61, 0x85, 0x09, 0xED, 0x3F, 0xDB, 0x57, 0xB3}
    };
    register v16qu XL, XH;
    register v16qu t0L, t0H, t1L, t1H;

    t0L   = ((v16qu)X) & V(0xFF, 0x00);
    t1H   = ((v16qu)X) & V(0x00, 0xFF);

    t1L   = (v16qu)__builtin_ia32_psrlwi128((v8hi)X, 8); //X  >> 8;
    t0H   = (v16qu)__builtin_ia32_psllwi128((v8hi)X, 8); //X  << 8;

    XL    = __builtin_shuffle(tables[0], t0L);
    XH    = __builtin_shuffle(tables[1], t0H);

    t0L   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t0L, 4) & V(0xFF, 0x00);
    t0H   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t0H, 4) & V(0x00, 0xFF);

    XL   ^= __builtin_shuffle(tables[2], t0L);
    XH   ^= __builtin_shuffle(tables[3], t0H);

    XL   ^= __builtin_shuffle(tables[4], t1L);
    XH   ^= __builtin_shuffle(tables[5], t1H);

    t1L   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t1L, 4) & V(0xFF, 0x00);
    t1H   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t1H, 4) & V(0x00, 0xFF);

    XL   ^= __builtin_shuffle(tables[6], t1L);
    XH   ^= __builtin_shuffle(tables[7], t1H);

    return (v8hu)(XL ^ XH);
}



/***
 * LLayerInv: Performs inverse linear transformation in one block using the L-box. 
 * The 128-bits register contain one block.
 * Input: 'X'_i in {0, 1, ..., 7} where X_i have 16-bits.
 * Output: 'return value' with L-box applied.
 */
static v8hu LLayerInv(v8hu X) {
    static const v16qu tables[8] =
    {
        {0x00, 0x65, 0x35, 0x50, 0x95, 0xF0, 0xA0, 0xC5, 0x89, 0xEC, 0xBC, 0xD9, 0x1C, 0x79, 0x29, 0x4C},
        {0x00, 0xB0, 0x89, 0x39, 0x4A, 0xFA, 0xC3, 0x73, 0xB4, 0x04, 0x3D, 0x8D, 0xFE, 0x4E, 0x77, 0xC7},
        {0x00, 0x57, 0xC5, 0x92, 0x89, 0xDE, 0x4C, 0x1B, 0x73, 0x24, 0xB6, 0xE1, 0xFA, 0xAD, 0x3F, 0x68},
        {0x00, 0x28, 0x26, 0x0E, 0xC9, 0xE1, 0xEF, 0xC7, 0x11, 0x39, 0x37, 0x1F, 0xD8, 0xF0, 0xFE, 0xD6},
        {0x00, 0xAD, 0x53, 0xFE, 0x0B, 0xA6, 0x58, 0xF5, 0x83, 0x2E, 0xD0, 0x7D, 0x88, 0x25, 0xDB, 0x76},
        {0x00, 0x82, 0x46, 0xC4, 0x55, 0xD7, 0x13, 0x91, 0x6C, 0xEE, 0x2A, 0xA8, 0x39, 0xBB, 0x7F, 0xFD},
        {0x00, 0x39, 0x03, 0x3A, 0xFE, 0xC7, 0xFD, 0xC4, 0x69, 0x50, 0x6A, 0x53, 0x97, 0xAE, 0x94, 0xAD},
        {0x00, 0x43, 0xB9, 0xFA, 0xFF, 0xBC, 0x46, 0x05, 0x16, 0x55, 0xAF, 0xEC, 0xE9, 0xAA, 0x50, 0x13}
    };
    register v16qu XL, XH;
    register v16qu t0L, t0H, t1L, t1H;

    t0L   = ((v16qu)X) & V(0xFF, 0x00);
    t1H   = ((v16qu)X) & V(0x00, 0xFF);

    t1L   = (v16qu)__builtin_ia32_psrlwi128((v8hi)X, 8); //X  >> 8;
    t0H   = (v16qu)__builtin_ia32_psllwi128((v8hi)X, 8); //X  << 8;

    XL    = __builtin_shuffle(tables[0], t0L);
    XH    = __builtin_shuffle(tables[1], t0H);

    t0L   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t0L, 4) & V(0xFF, 0x00);
    t0H   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t0H, 4) & V(0x00, 0xFF);

    XL   ^= __builtin_shuffle(tables[2], t0L);
    XH   ^= __builtin_shuffle(tables[3], t0H);

    XL   ^= __builtin_shuffle(tables[4], t1L);
    XH   ^= __builtin_shuffle(tables[5], t1H);

    t1L   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t1L, 4) & V(0xFF, 0x00);
    t1H   = (v16qu)__builtin_ia32_psrlwi128((v8hi)t1H, 4) & V(0x00, 0xFF);

    XL   ^= __builtin_shuffle(tables[6], t1L);
    XH   ^= __builtin_shuffle(tables[7], t1H);

    return (v8hu)(XL ^ XH);
}


/***
 * enc_fantomas: encrypt fantomas with both inputs and output aligned in 16. 
 * Input: 'in' the block with plaintext.
 *        'key' key to encrypt
 * Output: 'out' the block with ciphertext.
 */
void enc_fantomas(uint8_t *out, uint8_t *in, uint8_t *key) {
    register int32_t i;
    uint16_t Constant[12] = {
             0xBFFF, 0x6E90, 0xD16F, 0x4137, 0xFEC8, 0x2FA7, 
             0x9058, 0xD548, 0x6AB7, 0xBBD8, 0x0427, 0x947F};
    v8hu  *out_128 = (v8hu *)out, 
          *in_128  = (v8hu *)in, 
          *key_128 = (v8hu *)key; 

    //copy input and add key
    *out_128 = *key_128 ^ *in_128;

    //rounds
    for(i=0; i<12; i++) {
        //Round Constant Layer
        (*out_128)[0] ^= Constant[i];

        //S-Box Layer
        SLayer((*out_128));

        //L-Box Layer
        *out_128 = LLayer(*out_128);

        //Key Layer
        *out_128 ^= *key_128;
    }
}


/***
 * dec_fantomas: decrypt fantomas with both inputs and output aligned in 16. 
 * Input: 'in' the block with ciphertext.
 *        'key' key to encrypt
 * Output: 'out' the block with plaintext.
 */
void dec_fantomas(uint8_t *out, uint8_t *in, uint8_t *key) {
    register int32_t i;
    uint16_t Constant[12] = {
         0xBFFF, 0x6E90, 0xD16F, 0x4137, 0xFEC8, 0x2FA7, 
         0x9058, 0xD548, 0x6AB7, 0xBBD8, 0x0427, 0x947F};
    v8hu  *out_128 = (v8hu *)out, 
          *in_128  = (v8hu *)in, 
          *key_128 = (v8hu *)key; 

    *out_128 = *in_128;

    //rounds
    for(i=11; i>=0; i--) {
        //Key Layer
        *out_128 ^= *key_128;

        //Reverse L-Box Layer
        *out_128 = LLayerInv(*out_128);

        //Reverse S-Box Layer
        SLayerInv((*out_128));

        //Round Constant Layer
        (*out_128)[0] ^= Constant[i];
    }

    //copy add key
    *out_128 ^= *key_128;
}

