/*
 * scatterv.h: Benchmark functions for Scatterv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SCATTERV_H
#define SCATTERV_H

#include "bench.h"

int bench_scatterv_init(bench_t *bench);
int bench_scatterv_free(bench_t *bench);
int bench_scatterv_init_test(bench_t *bench);
int bench_scatterv_free_test(bench_t *bench);
int bench_scatterv_printinfo(bench_t *bench);
double measure_scatterv_sync(bench_t *bench);

#endif /* SCATTERV_H */
