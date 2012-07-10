/*
 * allgather.h: Benchmark functions for Allgather.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLGATHER_H
#define ALLGATHER_H

#include "bench_coll.h"

int bench_allgather_init(colltest_params_t *params);
int bench_allgather_free();
int bench_allgather_printinfo();
int measure_allgather_sync(colltest_params_t *params, double *time);

#endif /* ALLGATHER_H */
