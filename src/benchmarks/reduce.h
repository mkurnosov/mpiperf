/*
 * reduce.h: Benchmark functions for reduce.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REDUCE_H
#define REDUCE_H

#include "bench.h"

int bench_reduce_init(bench_t *bench);
int bench_reduce_free(bench_t *bench);
int bench_reduce_init_test(bench_t *bench);
int bench_reduce_free_test(bench_t *bench);
int bench_reduce_printinfo(bench_t *bench);
double measure_reduce_sync(bench_t *bench);

#endif /* REDUCE_H */
