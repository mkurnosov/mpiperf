/*
 * alltoallv.h: Benchmark functions for Alltoallv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLTOALLV_H
#define ALLTOALLV_H

#include "bench_coll.h"

int bench_alltoallv_init(colltest_params_t *params);
int bench_alltoallv_free();
int bench_alltoallv_printinfo();
int measure_alltoallv_sync(colltest_params_t *params, double *time);

#endif /* ALLTOALLV_H */
