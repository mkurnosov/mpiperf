/*
 * ialltoall.h: Benchmark functions for Ialltoall.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IALLTOALL_H
#define IALLTOALL_H

#include "bench_nbc.h"

int bench_ialltoall_init(nbctest_params_t *params);
int bench_ialltoall_free();
int bench_ialltoall_printinfo();
int measure_ialltoall_blocking(nbctest_params_t *params,
		                       nbctest_result_t *result);
int measure_ialltoall_overlap(nbctest_params_t *params,
		                      nbctest_result_t *result);

#endif /* IALLTOALL_H */
