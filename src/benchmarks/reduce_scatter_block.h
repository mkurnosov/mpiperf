/*
 * reduce_scatter_block.h: Benchmark functions for Reduce_scatter_block.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REDUCE_SCATTER_BLOCK_H
#define REDUCE_SCATTER_BLOCK_H

#include "bench.h"

int bench_reduce_scatter_block_init(bench_t *bench);
int bench_reduce_scatter_block_free(bench_t *bench);
int bench_reduce_scatter_block_init_test(bench_t *bench);
int bench_reduce_scatter_block_free_test(bench_t *bench);
int bench_reduce_scatter_block_printinfo(bench_t *bench);
double measure_reduce_scatter_block_sync(bench_t *bench);

#endif /* REDUCE_SCATTER_BLOCK_H */
