/*
 * allgather.h: Benchmark functions for Allgather.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLGATHER_H
#define ALLGATHER_H

#include "bench.h"

int bench_allgather_init(bench_t *bench);
int bench_allgather_free(bench_t *bench);
int bench_allgather_init_test(bench_t *bench);
int bench_allgather_free_test(bench_t *bench);
int bench_allgather_printinfo(bench_t *bench);
double measure_allgather_sync(bench_t *bench);

#endif /* ALLGATHER_H */
