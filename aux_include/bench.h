/*
* ======================================================================
* Copyright 2016 LG Electronics and  University of Campinas, All Rights 
* Reserved. The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ======================================================================
*/

#ifndef BENCH_H
#define BENCH_H

/*============================================================================*/
/* Constant definitions                                                       */
/*============================================================================*/

/**
 * Number of times to run benchmarks.
 */
#define BENCH       50

/*============================================================================*/
/* Macro definitions                                                          */
/*============================================================================*/

/**
 * Prints the last benchmark with a cycle-per-byte metric.
 */
#define BENCH_PRINT_CPB printf("%lu.%u%u cycles per byte\r\n", bench_total(), bench_dec(), bench_cen());


/**
 * Runs a new benchmark.
 *
 * @param[in] LABEL         - the label for this benchmark.
 */
#define BENCH_BEGIN(LABEL)                                                  \
    printf("BENCH: " LABEL " = ");                                          \
    bench_reset();                                                          \
    for (int i = 0; i < BENCH; i++) {                                       \


/**
 * Prints the mean timing of each execution in cycles per byte.
 */
#define BENCH_CPB(L)                                                        \
    }                                                                       \
    bench_compute_cpb(BENCH * BENCH * L);                                   \
    BENCH_PRINT_CPB;                                                        \

/**
 * Measures the time of one execution and adds it to the benchmark total.
 *
 * @param[in] FUNCTION      - the function executed.
 */
#define BENCH_ADD(FUNCTION)                                                 \
    FUNCTION;                                                               \
    bench_before();                                                         \
    for (int j = 0; j < BENCH; j++) {                                       \
        FUNCTION;                                                           \
    }                                                                       \
    bench_after();                                                          \

/*============================================================================*/
/* Function prototypes                                                        */
/*============================================================================*/

/**
 * Resets the benchmark data.
 *
 * @param[in] label         - the benchmark label.
 */
void bench_reset(void);

/**
 * Measures the time before a benchmark is executed.
 */
void bench_before(void);

/**
 * Measures the time after a benchmark was started and adds it to the total.
 */
void bench_after(void);

/**
 * Computes the mean elapsed time between the start and the end of a benchmark.
 *
 * @param benches           - the number of executed benchmarks.
 */
void bench_compute(int benches);

/**
 * Computes the mean elapsed time between the start and the end of a benchmark.
 *
 * @param benches           - the number of executed benchmarks.
 */
void bench_compute_cpb(int benches);

/**
 * Returns the result of the last benchmark.
 *
 * @return the last benchmark.
 */
unsigned long bench_total(void);

unsigned int bench_dec(void);

unsigned int bench_cen(void);

#include <stdint.h>

int rand_bytes(uint8_t *buf, int size);

#endif /* !BENCH_H */
