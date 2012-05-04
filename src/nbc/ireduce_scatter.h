/*
 * ireduce_scatter.h: Benchmark functions for Ireduce_scatter.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IREDUCE_SCATTER_H
#define IREDUCE_SCATTER_H

#include "bench_nbc.h"

int bench_ireduce_scatter_init(nbctest_params_t *params);
int bench_ireduce_scatter_free();
int bench_ireduce_scatter_printinfo();
int measure_ireduce_scatter_blocking(nbctest_params_t *params,
		                             nbctest_result_t *result);
int measure_ireduce_scatter_overlap(nbctest_params_t *params,
		                            nbctest_result_t *result);

#endif /* IREDUCE_SCATTER_H */
