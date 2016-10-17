/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#include "fantomas16_ctr.h"
#include <tmmintrin.h> //SSSE3

typedef    uint8_t    v16qu  __attribute__ ((vector_size (16)));
typedef       char    v16qi  __attribute__ ((vector_size (16)));
typedef    int32_t     v4si  __attribute__ ((vector_size (16)));

#define V(x) (v16qu) {x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x}


/***
 * SLayer: Performs substitution in one part of 16 simultaneous blocks using Misty 3/5 bits. 
 * Each 128-bits register contains one byte that is in the same position of each block.
 * Input: 'X'_i in {0, 1, ..., 7} where X_i have 128 bits.
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
 * LLayer: Performs linear transformation in 16 simultaneous blocks using the L-box. 
 * Each 128-bits register contains one byte that is in the same position of each block.
 * L-box:
 *      10110000011001011011000001100101
 *      10001001001101011000100100110101
 *      01001010100101010100101010010101
 *      10110100100010011011010010001001
 *      00101000010101110010100001010111
 *      00100110110001010010011011000101
 *      11001001100010011100100110001001
 *      00010001011100110001000101110011
 *      10000010101011011000001010101101
 *      01000110010100110100011001010011
 *      01010101000010110101010100001011
 *      01101100100000110110110010000011
 *      01000011001110010100001100111001
 *      10111001000000111011100100000011
 *      11111111111111101111111111111110
 *      00010110011010010001011001101001 
 * Input: 'X'_i in {0, 1, ..., 7} where X_i have 128 bits.
 * Output: 'X'_i with L-box applied.
 */
static inline void LLayer(v16qu *X) {
    register int i;
    static const v16qu tables[8] = //Put L-box em register tables
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

    //process 2 block in parallel
    for(i=0; i<8; i+=2) {
        v16qu t[4] = {X[i], X[i+8], X[i+1], X[i+9]}; //X[i] contain the byte less significant of the block i 
                                                     //X[i+8] contain the byte most significatn of the block i

        //replace the 4 bits less significant of the t[0]/t[2]
        X[i]    = __builtin_shuffle(tables[0], t[0]); 
        X[i+1]  = __builtin_shuffle(tables[0], t[2]);

        X[i+8]  = __builtin_shuffle(tables[1], t[0]);
        X[i+9]  = __builtin_shuffle(tables[1], t[2]);

        t[0]    = (v16qu)__builtin_ia32_psrldi128((v4si)t[0], 4);
        t[2]    = (v16qu)__builtin_ia32_psrldi128((v4si)t[2], 4);

        //replace the 4 bits most significant of the t[0]/t[2]
        X[i]   ^= __builtin_shuffle(tables[2], t[0]);
        X[i+1] ^= __builtin_shuffle(tables[2], t[2]);

        X[i+8] ^= __builtin_shuffle(tables[3], t[0]);
        X[i+9] ^= __builtin_shuffle(tables[3], t[2]);

        //similarly with the byte most significant
        X[i]   ^= __builtin_shuffle(tables[4], t[1]);
        X[i+1] ^= __builtin_shuffle(tables[4], t[3]);

        X[i+8] ^= __builtin_shuffle(tables[5], t[1]);
        X[i+9] ^= __builtin_shuffle(tables[5], t[3]);

        t[1]    = (v16qu)__builtin_ia32_psrldi128((v4si)t[1], 4);;
        t[3]    = (v16qu)__builtin_ia32_psrldi128((v4si)t[3], 4);;

        X[i]   ^= __builtin_shuffle(tables[6], t[1]);
        X[i+1] ^= __builtin_shuffle(tables[6], t[3]);

        X[i+8] ^= __builtin_shuffle(tables[7], t[1]);
        X[i+9] ^= __builtin_shuffle(tables[7], t[3]);
    }
}


/***
 * enc_fantomas16: Performs encrypt of 16 blocks in parallel. 
 * Each 128-bits register contains one byte that is in the same position of each block.
 * Input: 'in'_i in {0, 1, ..., 7} where in_i have 128 bits.
 *        'key'_i in {0, 1, ..., 7} where key_i have 128 bits.
 * Output: 'out'_i in {0, 1, ..., 7}, contain the encrypt of 'in' with the 'key'.
 */
static void enc_fantomas16(v16qu *out, v16qu *in, v16qu *key) {
    //Rounds constants
    static const uint8_t ConstantH[12] = {0xBF, 0x6E, 0xD1, 0x41, 0xFE, 0x2F, 0x90, 0xD5, 0x6A, 0xBB, 0x04, 0x94};
    static const uint8_t ConstantL[12] = {0xFF, 0x90, 0x6F, 0x37, 0xC8, 0xA7, 0x58, 0x48, 0xB7, 0xD8, 0x27, 0x7F};
    register int32_t i, j;

    for(i=0; i<16; i++) { //Copy the input with add key
        out[i] = in[i] ^ key[i];
    }

    //rounds
    for(i=0; i<12; i++) {
        //Round Constant Layer
        out[0] ^= V(ConstantL[i]); // constant in the byte less significant
        out[8] ^= V(ConstantH[i]); // constant in the byte most significant

        //S-Box Layer
        SLayer(out); //Apply the S-box in the bytes less significant of each 16-bits word
        SLayer((out+8)); //Apply the S-box in the bytes most significant of each 16-bits word

        //L-Box Layer
        LLayer(out);

        //Key Layer
        for(j=0; j<16; j++) {
            out[j] ^= key[j];
        }
    }
}


/***
 * __interleave: Interleave two vectors.
 * Input: 'x', 'y'  vectors {x0, x1, ..., x14, x15, y0, y1, ..., y14, y15}.
 * Output: 'x', 'y'  vectors {x0, y0, ..., x7, y7, x8, y8, ..., x15, y15}.
 */
#define __interleave(X,Y) {                         \
    v16qi _X = (v16qi)X, _Y = (v16qi)Y;             \
    X = (v16qu)__builtin_ia32_punpcklbw128(_X, _Y); \
    Y = (v16qu)__builtin_ia32_punpckhbw128(_X, _Y); \
}


/***
 * __transpose: transpose a matrix 16 x 16 using interleave.
 * Input: matrix 'X'.
 * Output: matrix 'X' transpose.
 */
#define __transpose(X0,X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,X13,X14,X15) { \
    __interleave( X0,  X8);                                                  \
    __interleave( X1,  X9);                                                  \
    __interleave( X2, X10);                                                  \
    __interleave( X3, X11);                                                  \
    __interleave( X4, X12);                                                  \
    __interleave( X5, X13);                                                  \
    __interleave( X6, X14);                                                  \
    __interleave( X7, X15);                                                  \
                                                                             \
    __interleave( X0,  X4);                                                  \
    __interleave( X1,  X5);                                                  \
    __interleave( X2,  X6);                                                  \
    __interleave( X3,  X7);                                                  \
    __interleave( X8, X12);                                                  \
    __interleave( X9, X13);                                                  \
    __interleave(X10, X14);                                                  \
    __interleave(X11, X15);                                                  \
                                                                             \
    __interleave( X0,  X2);                                                  \
    __interleave( X1,  X3);                                                  \
    __interleave( X4,  X6);                                                  \
    __interleave( X5,  X7);                                                  \
    __interleave( X8, X10);                                                  \
    __interleave( X9, X11);                                                  \
    __interleave(X12, X14);                                                  \
    __interleave(X13, X15);                                                  \
                                                                             \
    __interleave( X0,  X1);                                                  \
    __interleave( X2,  X3);                                                  \
    __interleave( X4,  X5);                                                  \
    __interleave( X6,  X7);                                                  \
    __interleave( X8,  X9);                                                  \
    __interleave(X10, X11);                                                  \
    __interleave(X12, X13);                                                  \
    __interleave(X14, X15);                                                  \
}


/***
 * __carry_prop: do the propagation of the carry in the counter mode 
 * Input: 'X' with the actual state of the counter.
 *        'C' the carry of the actual counter.
 *        'T' temporary aux.
 * Output: 'X' with the result post applied carry.
 *         'C' the next carry.
 */
#define __carry_prop(X, C, T) { \
    T = C + X;                  \
    C = (T < X) & V(0x01);      \
    X = T;                      \
}


/***
 * __carry_last: do the propagation of the last carry in the counter mode 
 * Input: 'X' with the actual state of the counter.
 *        'C' the carry of the actual counter.
 * Output: 'X' with the result post applied carry.
 */
#define __carry_last(X, C) {  \
    X += C;                   \
}


/***
 * copy_counter16: copy and extend the counter in the init
 * Input: 'init' the original counter.
 * Output: 'cnt' counter extend to 16 blocks.
 */
static inline void copy_counter16(v16qu *cnt, uint8_t *init) {
    v16qu a, b;
    //update the counter 
    static const v16qu add = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    register v16qu
        X0  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X1  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X2  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X3  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X4  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X5  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X6  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X7  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X8  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X9  = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X10 = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X11 = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X12 = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X13 = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X14 = (v16qu)__builtin_ia32_loaddqu((const char *)init),
        X15 = (v16qu)__builtin_ia32_loaddqu((const char *)init);

    //transpose the counters to use in fantomas
    __transpose(X0,X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,X13,X14,X15);

    a = X0; //save X0 and X1 (have only 16 SSE register)
    b = X1;
    X1 = add;

    //add in the counter and propagate the carry
    __carry_prop(X15, X1, X0);
    __carry_prop(X14, X1, X0);
    __carry_prop(X13, X1, X0);
    __carry_prop(X12, X1, X0);
    __carry_prop(X11, X1, X0);
    __carry_prop(X10, X1, X0);
    __carry_prop( X9, X1, X0);
    __carry_prop( X8, X1, X0);
    __carry_prop( X7, X1, X0);
    __carry_prop( X6, X1, X0);
    __carry_prop( X5, X1, X0);
    __carry_prop( X4, X1, X0);
    __carry_prop( X3, X1, X0);
    __carry_prop( X2, X1, X0);


    //save the counter cnt[0 .. 7] have  X0, X2, ..., X14 --> less significant bytes of fantomas block
    // and cnt[8 .. 15] have X1, X3, ..., X15 --> most significant bytes of the fantomas block
    cnt[15] = X15;
    cnt[14] = X13;

    X15 = X1;
    X1 = b;
    X0 = a;

    __carry_prop(X14, X15 , X13 );
    __carry_last(X15, X15);

    cnt[13] = X11;
    cnt[12] = X9;
    cnt[11] = X7;
    cnt[10] = X5;
    cnt[9]  = X3;
    cnt[8]  = X1;
    cnt[7]  = X14;
    cnt[6]  = X12;
    cnt[5]  = X10;
    cnt[4]  = X8;
    cnt[3]  = X6;
    cnt[2]  = X4;
    cnt[1]  = X2;
    cnt[0]  = X0;
}


/***
 * extend_key16: extend the key in the init to 16 blocks.
 * Input: 'init' the original key.
 * Output: 'key' extend key to 16 blocks.
 */
static inline void extend_key16(v16qu *key, uint8_t *init) {
    register int i;

    for(i=0; i<8; i++) {
        key[i] = V(init[2*i]);
        key[i+8] = V(init[2*i+1]);
    }
}


/***
 * update_counter16: update the 16 counters
 * Input: 'cnt' counter actual.
 * Output: 'cnt' update.
 */
static inline void update_counter16(v16qu *cnt) {
    v16qu a, b;
    static const v16qu add = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    register v16qu
        X0  = cnt[0],
        X2  = cnt[1],
        X4  = cnt[2],
        X6  = cnt[3],
        X8  = cnt[4],
        X10 = cnt[5],
        X12 = cnt[6],
        X14 = cnt[7],
        X1  = cnt[8],
        X3  = cnt[9],
        X5  = cnt[10],
        X7  = cnt[11],
        X9  = cnt[12],
        X11 = cnt[13],
        X13 = cnt[14],
        X15 = cnt[15];

    a = X0;
    b = X1;
    X1 = add;

    __carry_prop(X15, X1, X0);
    __carry_prop(X14, X1, X0);
    __carry_prop(X13, X1, X0);
    __carry_prop(X12, X1, X0);
    __carry_prop(X11, X1, X0);
    __carry_prop(X10, X1, X0);
    __carry_prop( X9, X1, X0);
    __carry_prop( X8, X1, X0);
    __carry_prop( X7, X1, X0);
    __carry_prop( X6, X1, X0);
    __carry_prop( X5, X1, X0);
    __carry_prop( X4, X1, X0);
    __carry_prop( X3, X1, X0);
    __carry_prop( X2, X1, X0);

    cnt[15] = X15;
    cnt[14] = X13;

    X15 = X1;
    X1 = b;
    X0 = a;

    __carry_prop(X1, X15 , X13);
    __carry_last(X0, X15);

    cnt[13] = X11;
    cnt[12] = X9;
    cnt[11] = X7;
    cnt[10] = X5;
    cnt[9]  = X3;
    cnt[8]  = X1;
    cnt[7]  = X14;
    cnt[6]  = X12;
    cnt[5]  = X10;
    cnt[4]  = X8;
    cnt[3]  = X6;
    cnt[2]  = X4;
    cnt[1]  = X2;
    cnt[0]  = X0;
}


/***
 * transpose16: transpose using registers. Remember the matrix is separete in a diferent way.
 * Input: 'val' matrix before transpose
 * Output: 'val' matrix after transpose
 */
static inline void transpose16(v16qu *val) {
    register v16qu
        X0  = val[0],
        X1  = val[8],
        X2  = val[1],
        X3  = val[9],
        X4  = val[2],
        X5  = val[10],
        X6  = val[3],
        X7  = val[11],
        X8  = val[4],
        X9  = val[12],
        X10 = val[5],
        X11 = val[13],
        X12 = val[6],
        X13 = val[14],
        X14 = val[7],
        X15 = val[15];

    __transpose(X0,X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12,X13,X14,X15);

    val[0]  = X0;
    val[1]  = X1;
    val[2]  = X2;
    val[3]  = X3;
    val[4]  = X4;
    val[5]  = X5;
    val[6]  = X6;
    val[7]  = X7;
    val[8]  = X8;
    val[9]  = X9;
    val[10] = X10;
    val[11] = X11;
    val[12] = X12;
    val[13] = X13;
    val[14] = X14;
    val[15] = X15;
}

/***
 * ctr_fantomas16: using the CTR with 16 blocks of fantomas in parallel
 * Input: 'io' ciphertex/plaintext, 
 *        'l_io' the size of the io
 *        'k'  the key to encrypt/decrypt
 *        'c' the counter to encrypt/decrypt  
 * Output: 'io' plaintext/ciphertex.
 */
void ctr_fantomas16(uint8_t *io, uint32_t l_io, uint8_t *k, uint8_t *c) {
    v16qu ct[16], out[16], key[16];
    uint8_t cs[256] __attribute__ ((aligned (16)));
    v16qu *io_128 = (v16qu *)io, *cs_128 = (v16qu *)cs;
    uint32_t blocks = l_io >> 8;
    register uint32_t i,j;

    copy_counter16(ct,c);
    extend_key16(key,k);

    //complet blocks
    for(i=0; i<blocks; i++) {
        enc_fantomas16(out, ct, key);
        update_counter16(ct);
        transpose16(out); //transpose to add the block
        for(j=0; j<16; j++) { //copy output
            io_128[(i << 4)+j] ^= out[j];
        }
    }

    //need ciphertext stealing:
    if(l_io % 256 != 0) {
        enc_fantomas16(out, ct, key);
        transpose16(out);
        for(i=0; i<=(l_io % 256) >> 4; i++) {
            cs_128[i] = out[i];
        }
        for(i=0; i<l_io % 256; i++) {
            io[blocks*256 + i] ^= cs[i];
        }
    }
}

