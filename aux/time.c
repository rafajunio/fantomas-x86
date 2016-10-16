/*
* ====================================================================
* Copyright 2016 LG Electronics, All Rights Reserved.
* The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ====================================================================
*/

#include <stdio.h>
#include "bench.h"
#include "modes.h"
#include "fantomas.h"

#define M_SIZE  1024

void time_enc_cbc_fantomas_sse( ) {
    uint32_t l_io;
    uint8_t io[M_SIZE] __attribute__ ((aligned (16))), 
            k[16] __attribute__ ((aligned (16))), 
            iv[16] __attribute__ ((aligned (16)));

    rand_bytes(io, M_SIZE);
    rand_bytes(iv, 16);
    rand_bytes(k, 16);

    BENCH_BEGIN("Encrypt Fantomas SSE + CBC") {
        l_io = M_SIZE;
        rand_bytes(io, M_SIZE);
        rand_bytes(k, 16);
        rand_bytes(iv, 16);
        BENCH_ADD(enc_cbc128_raw(io, l_io, enc_fantomas, k, iv));
    } BENCH_CPB(M_SIZE);
}


void time_dec_cbc_fantomas_sse( ) {
    uint32_t l_io;
     uint8_t io[M_SIZE] __attribute__ ((aligned (16))), 
            k[16] __attribute__ ((aligned (16))), 
            iv[16] __attribute__ ((aligned (16)));

    rand_bytes(io, M_SIZE);
    rand_bytes(iv, 16);
    rand_bytes(k, 16);

    BENCH_BEGIN("Decrypt Fantomas SSE + CBC") {
        l_io = M_SIZE;
        rand_bytes(io, M_SIZE);
        rand_bytes(k, 16);
        rand_bytes(iv, 16);
        BENCH_ADD(dec_cbc128_raw(io, l_io, dec_fantomas, k, iv));
    } BENCH_CPB(M_SIZE);
}


void time_enc_dec_ctr_fantomas_sse( ) {
    uint32_t l_io;
    uint8_t io[M_SIZE] __attribute__ ((aligned (16))), 
            k[16] __attribute__ ((aligned (16))), 
            c[16] __attribute__ ((aligned (16)));

    rand_bytes(io, M_SIZE);
    rand_bytes(c, 16);
    rand_bytes(k, 16);

    BENCH_BEGIN("Encrypt/Decrypt Fantomas SSE + CTR") {
        l_io = M_SIZE;
        rand_bytes(io, M_SIZE);
        rand_bytes(k, 16);
        rand_bytes(c, 16);
        BENCH_ADD(ctr128_raw(io,l_io, enc_fantomas,k, c));
    } BENCH_CPB(M_SIZE);
}


void time_enc_dec_fantomas16CTR_sse( ) {
    uint32_t l_io;
    uint8_t io[M_SIZE] __attribute__ ((aligned (16))), 
            k[16] __attribute__ ((aligned (16))), 
            c[16] __attribute__ ((aligned (16)));

    rand_bytes(io, M_SIZE);
    rand_bytes(c, 16);
    rand_bytes(k, 16);

    BENCH_BEGIN("Encrypt/Decrypt Fantomas SSE + 16 blocks CTR") {
        l_io = M_SIZE;
        rand_bytes(io, M_SIZE);
        rand_bytes(k, 16);
        rand_bytes(c, 16);
        BENCH_ADD(ctr_fantomas16(io,l_io, k, c));
    } BENCH_CPB(M_SIZE);
}


int main( ) {
    time_enc_cbc_fantomas_sse( );
    time_dec_cbc_fantomas_sse( );
    time_enc_dec_ctr_fantomas_sse( );
    time_enc_dec_fantomas16CTR_sse( );

    return 0;
}