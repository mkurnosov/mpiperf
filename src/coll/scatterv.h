/*
 * scatterv.h: Benchmark functions for Scatterv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SCATTERV_H
#define SCATTERV_H

#include "bench_coll.h"

int bench_scatterv_init(colltest_params_t *params);
int bench_scatterv_free();
int bench_scatterv_printinfo();
int measure_scatterv_sync(colltest_params_t *params, double *time);

#endif /* SCATTERV_H */
