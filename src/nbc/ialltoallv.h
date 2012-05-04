/*
 * ialltoallv.h: Benchmark functions for Ialltoallv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IALLTOALLV_H
#define IALLTOALLV_H

#include "bench_nbc.h"

int bench_ialltoallv_init(nbctest_params_t *params);
int bench_ialltoallv_free();
int bench_ialltoallv_printinfo();
int measure_ialltoallv_blocking(nbctest_params_t *params,
		                        nbctest_result_t *result);
int measure_ialltoallv_overlap(nbctest_params_t *params,
		                       nbctest_result_t *result);

#endif /* IALLTOALLV_H */
