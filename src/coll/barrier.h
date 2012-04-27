/*
 * barrier.h: Benchmark functions for Barrier.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef BARRIER_H
#define BARRIER_H

#include "bench_coll.h"

int bench_barrier_init(colltest_params_t *params);
int bench_barrier_free();
int bench_barrier_printinfo();
int measure_barrier_sync(colltest_params_t *params, double *time);

#endif /* BARRIER_H */
