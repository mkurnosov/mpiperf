/*
 * reduce_scatter.h: Benchmark functions for Reduce_scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REDUCE_SCATTER_H
#define REDUCE_SCATTER_H

#include "bench_coll.h"

int bench_reduce_scatter_init(colltest_params_t *params);
int bench_reduce_scatter_free();
int bench_reduce_scatter_printinfo();
int measure_reduce_scatter_sync(colltest_params_t *params, double *time);

#endif /* REDUCE_SCATTER_H */
