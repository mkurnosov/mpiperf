/*
 * gather.h: Benchmark functions for Gather.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef GATHER_H
#define GATHER_H

#include "bench_coll.h"

int bench_gather_init(colltest_params_t *params);
int bench_gather_free();
int bench_gather_printinfo();
int measure_gather_sync(colltest_params_t *params, double *time);

#endif /* GATHER_H */
