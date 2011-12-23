/*
 * alltoallw.h: Benchmark functions for Alltoallw.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLTOALLW_H
#define ALLTOALLW_H

#include "bench.h"

int bench_alltoallw_init(bench_t *bench);
int bench_alltoallw_free(bench_t *bench);
int bench_alltoallw_init_test(bench_t *bench);
int bench_alltoallw_free_test(bench_t *bench);
int bench_alltoallw_printinfo(bench_t *bench);
double measure_alltoallw_sync(bench_t *bench);

#endif /* ALLTOALLW_H */
