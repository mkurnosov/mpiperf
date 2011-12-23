/*
 * barrier.h: Benchmark functions for Barrier.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef BARRIER_H
#define BARRIER_H

#include "bench.h"

int bench_barrier_init(bench_t *bench);
int bench_barrier_free(bench_t *bench);
int bench_barrier_init_test(bench_t *bench);
int bench_barrier_free_test(bench_t *bench);
int bench_barrier_printinfo(bench_t *bench);
double measure_barrier_sync(bench_t *bench);

#endif /* BARRIER_H */
