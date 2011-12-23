/*
 * alltoall.h: Benchmark functions for Alltoall.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLTOALL_H
#define ALLTOALL_H

#include "bench.h"

int bench_alltoall_init(bench_t *bench);
int bench_alltoall_free(bench_t *bench);
int bench_alltoall_init_test(bench_t *bench);
int bench_alltoall_free_test(bench_t *bench);
int bench_alltoall_printinfo(bench_t *bench);
double measure_alltoall_sync(bench_t *bench);

#endif /* ALLTOALL_H */
