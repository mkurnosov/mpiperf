/*
 * alltoallv.h: Benchmark functions for Alltoallv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLTOALLV_H
#define ALLTOALLV_H

#include "bench.h"

int bench_alltoallv_init(bench_t *bench);
int bench_alltoallv_free(bench_t *bench);
int bench_alltoallv_init_test(bench_t *bench);
int bench_alltoallv_free_test(bench_t *bench);
int bench_alltoallv_printinfo(bench_t *bench);
double measure_alltoallv_sync(bench_t *bench);

#endif /* ALLTOALLV_H */
