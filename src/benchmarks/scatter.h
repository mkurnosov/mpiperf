/*
 * scatter.h: Benchmark functions for scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SCATTER_H
#define SCATTER_H

#include "bench.h"

int bench_scatter_init(bench_t *bench);
int bench_scatter_free(bench_t *bench);
int bench_scatter_init_test(bench_t *bench);
int bench_scatter_free_test(bench_t *bench);
int bench_scatter_printinfo(bench_t *bench);
double measure_scatter_sync(bench_t *bench);

#endif /* SCATTER_H */
