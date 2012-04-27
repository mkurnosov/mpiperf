/*
 * alltoall.h: Benchmark functions for Alltoall.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLTOALL_H
#define ALLTOALL_H

#include "bench_coll.h"

int bench_alltoall_init(colltest_params_t *params);
int bench_alltoall_free();
int bench_alltoall_printinfo();
int measure_alltoall_sync(colltest_params_t *params, double *time);

#endif /* ALLTOALL_H */
