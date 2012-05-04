/*
 * iallgather.h: Benchmark functions for Iallgather.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IALLGATHER_H
#define IALLGATHER_H

#include "bench_nbc.h"

int bench_iallgather_init(nbctest_params_t *params);
int bench_iallgather_free();
int bench_iallgather_printinfo();
int measure_iallgather_blocking(nbctest_params_t *params,
		                        nbctest_result_t *result);
int measure_iallgather_overlap(nbctest_params_t *params,
		                       nbctest_result_t *result);

#endif /* IALLGATHER_H */
