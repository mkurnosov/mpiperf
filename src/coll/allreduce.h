/*
 * allreduce.h: Benchmark functions for Allreduce.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLREDUCE_H
#define ALLREDUCE_H

#include "bench_coll.h"

int bench_allreduce_init(colltest_params_t *params);
int bench_allreduce_free();
int bench_allreduce_printinfo();
int measure_allreduce_sync(colltest_params_t *params, double *time);

#endif /* ALLREDUCE_H */
