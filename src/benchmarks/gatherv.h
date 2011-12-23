/*
 * gatherv.h: Benchmark functions for Gatherv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef GATHERV_H
#define GATHERV_H

#include "bench.h"

int bench_gatherv_init(bench_t *bench);
int bench_gatherv_free(bench_t *bench);
int bench_gatherv_init_test(bench_t *bench);
int bench_gatherv_free_test(bench_t *bench);
int bench_gatherv_printinfo(bench_t *bench);
double measure_gatherv_sync(bench_t *bench);

#endif /* GATHERV_H */
