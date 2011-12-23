/*
 * allgatherv.h: Benchmark functions for Allgatherv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLGATHERV_H
#define ALLGATHERV_H

#include "bench.h"

int bench_allgatherv_init(bench_t *bench);
int bench_allgatherv_free(bench_t *bench);
int bench_allgatherv_init_test(bench_t *bench);
int bench_allgatherv_free_test(bench_t *bench);
int bench_allgatherv_printinfo(bench_t *bench);
double measure_allgatherv_sync(bench_t *bench);

#endif /* ALLGATHERV_H */
