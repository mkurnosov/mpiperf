/*
 * gather.h: Benchmark functions for Gather.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef GATHER_H
#define GATHER_H

#include "bench.h"

int bench_gather_init(bench_t *bench);
int bench_gather_free(bench_t *bench);
int bench_gather_init_test(bench_t *bench);
int bench_gather_free_test(bench_t *bench);
int bench_gather_printinfo(bench_t *bench);
double measure_gather_sync(bench_t *bench);

#endif /* GATHER_H */
