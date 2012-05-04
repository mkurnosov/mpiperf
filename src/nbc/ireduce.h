/*
 * ireduce.h: Benchmark functions for Ireduce.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IREDUCE_H
#define IREDUCE_H

#include "bench_nbc.h"

int bench_ireduce_init(nbctest_params_t *params);
int bench_ireduce_free();
int bench_ireduce_printinfo();
int measure_ireduce_blocking(nbctest_params_t *params,
		                     nbctest_result_t *result);
int measure_ireduce_overlap(nbctest_params_t *params,
		                    nbctest_result_t *result);

#endif /* IREDUCE_H */
