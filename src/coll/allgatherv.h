/*
 * allgatherv.h: Benchmark functions for Allgatherv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLGATHERV_H
#define ALLGATHERV_H

#include "bench_coll.h"

int bench_allgatherv_init(colltest_params_t *params);
int bench_allgatherv_free();
int bench_allgatherv_printinfo();
int measure_allgatherv_sync(colltest_params_t *params, double *time);

#endif /* ALLGATHERV_H */
