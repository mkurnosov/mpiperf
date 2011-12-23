/*
 * reduce_scatter.h: Benchmark functions for Reduce_scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REDUCE_SCATTER_H
#define REDUCE_SCATTER_H

#include "bench.h"

int bench_reduce_scatter_init(bench_t *bench);
int bench_reduce_scatter_free(bench_t *bench);
int bench_reduce_scatter_init_test(bench_t *bench);
int bench_reduce_scatter_free_test(bench_t *bench);
int bench_reduce_scatter_printinfo(bench_t *bench);
double measure_reduce_scatter_sync(bench_t *bench);

#endif /* REDUCE_SCATTER_H */
