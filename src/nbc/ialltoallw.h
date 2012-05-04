/*
 * ialltoallw.h: Benchmark functions for Ialltoallv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IALLTOALLW_H
#define IALLTOALLW_H

#include "bench_nbc.h"

int bench_ialltoallw_init(nbctest_params_t *params);
int bench_ialltoallw_free();
int bench_ialltoallw_printinfo();
int measure_ialltoallw_blocking(nbctest_params_t *params,
		                        nbctest_result_t *result);
int measure_ialltoallw_overlap(nbctest_params_t *params,
		                       nbctest_result_t *result);

#endif /* IALLTOALLW_H */
