/*
 * ibarrier.h: Benchmark functions for Ibarrier.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IBARRIER_H
#define IBARRIER_H

#include "bench_nbc.h"

int bench_ibarrier_init(nbctest_params_t *params);
int bench_ibarrier_free();
int bench_ibarrier_printinfo();
int measure_ibarrier_blocking(nbctest_params_t *params,
		                      nbctest_result_t *result);
int measure_ibarrier_overlap(nbctest_params_t *params,
		                     nbctest_result_t *result);

#endif /* IBARRIER_H */
