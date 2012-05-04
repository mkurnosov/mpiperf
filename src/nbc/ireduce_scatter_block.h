/*
 * ireduce_scatter_block.h: Benchmark functions for Ireduce_scatter_block.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IREDUCE_SCATTER_BLOCK_H
#define IREDUCE_SCATTER_BLOCK_H

#include "bench_nbc.h"

int bench_ireduce_scatter_block_init(nbctest_params_t *params);
int bench_ireduce_scatter_block_free();
int bench_ireduce_scatter_block_printinfo();
int measure_ireduce_scatter_block_blocking(nbctest_params_t *params,
		                                   nbctest_result_t *result);
int measure_ireduce_scatter_block_overlap(nbctest_params_t *params,
		                                  nbctest_result_t *result);

#endif /* IREDUCE_SCATTER_BLOCK_H */
