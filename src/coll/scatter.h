/*
 * scatter.h: Benchmark functions for scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SCATTER_H
#define SCATTER_H

#include "bench_coll.h"

int bench_scatter_init(colltest_params_t *params);
int bench_scatter_free();
int bench_scatter_printinfo();
int measure_scatter_sync(colltest_params_t *params, double *time);

#endif /* SCATTER_H */
