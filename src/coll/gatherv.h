/*
 * gatherv.h: Benchmark functions for Gatherv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef GATHERV_H
#define GATHERV_H

#include "bench_coll.h"

int bench_gatherv_init(colltest_params_t *params);
int bench_gatherv_free();
int bench_gatherv_printinfo();
int measure_gatherv_sync(colltest_params_t *params, double *time);

#endif /* GATHERV_H */
