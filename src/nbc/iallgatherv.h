/*
 * iallgatherv.h: Benchmark functions for Iallgatherv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IALLGATHERV_H
#define IALLGATHERV_H

#include "bench_nbc.h"

int bench_iallgatherv_init(nbctest_params_t *params);
int bench_iallgatherv_free();
int bench_iallgatherv_printinfo();
int measure_iallgatherv_blocking(nbctest_params_t *params,
		                         nbctest_result_t *result);
int measure_iallgatherv_overlap(nbctest_params_t *params,
		                        nbctest_result_t *result);

#endif /* IALLGATHERV_H */
