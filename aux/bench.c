/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#define asm                 __asm__ volatile

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


int rand_bytes(uint8_t *buf, int size) {
    int c, l, fd;
    
    fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        return 0;
    }

    l = 0;
    do {
        c = read(fd, buf + l, size - l);
        l += c;
        if (c == -1) {
            return l;
        }
    } while (l < size);
    
    close(fd);
    return l;
}

unsigned long cycles0(void) {
    unsigned int hi, lo;
    asm (
        "cpuid\n\t"/*serialize*/
        "rdtsc\n\t"/*read the clock*/
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        : "=r" (hi), "=r" (lo):: "%rax", "%rbx", "%rcx", "%rdx"
    );
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}

unsigned long cycles1(void) {
    unsigned int hi, lo;
    asm (
        "cpuid\n\t"/*serialize*/
        "rdtsc\n\t"/*read the clock*/
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        : "=r" (hi), "=r" (lo):: "%rax", "%rbx", "%rcx", "%rdx"
    );
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}

static unsigned long before, after, total;

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void bench_reset() {
    total = 0;
}

void bench_before() {
    before = cycles0();
}

void bench_after() {
    long long result;

    after = cycles1();
    result = (after - before);

    total += result;
}

static unsigned int dec, cen;


void bench_compute_cpb(int benches) {
    dec = ((total % benches)*10)/benches;
    cen = ((((total % benches)*10) % benches)*10)/benches;
    total = total/benches;
}

unsigned int bench_dec( ) {
    return dec;
}

unsigned int bench_cen( ) {
    return cen;
}

unsigned long bench_total() {
    return total;
}
