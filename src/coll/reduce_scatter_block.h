/*
 * reduce_scatter_block.h: Benchmark functions for Reduce_scatter_block.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REDUCE_SCATTER_BLOCK_H
#define REDUCE_SCATTER_BLOCK_H

#include "bench_coll.h"

int bench_reduce_scatter_block_init(colltest_params_t *params);
int bench_reduce_scatter_block_free();
int bench_reduce_scatter_block_printinfo();
int measure_reduce_scatter_block_sync(colltest_params_t *params, double *time);

#endif /* REDUCE_SCATTER_BLOCK_H */
