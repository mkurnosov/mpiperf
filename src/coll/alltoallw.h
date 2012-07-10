/*
 * alltoallw.h: Benchmark functions for Alltoallw.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLTOALLW_H
#define ALLTOALLW_H

#include "bench_coll.h"

int bench_alltoallw_init(colltest_params_t *params);
int bench_alltoallw_free();
int bench_alltoallw_printinfo();
int measure_alltoallw_sync(colltest_params_t *params, double *time);

#endif /* ALLTOALLW_H */
