/*
 * reduce.h: Benchmark functions for reduce.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REDUCE_H
#define REDUCE_H

#include "bench_coll.h"

int bench_reduce_init(colltest_params_t *params);
int bench_reduce_free();
int bench_reduce_printinfo();
int measure_reduce_sync(colltest_params_t *params, double *time);

#endif /* REDUCE_H */
