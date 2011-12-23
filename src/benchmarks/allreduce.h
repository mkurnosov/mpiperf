/*
 * allreduce.h: Benchmark functions for Allreduce.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ALLREDUCE_H
#define ALLREDUCE_H

#include "bench.h"

int bench_allreduce_init(bench_t *bench);
int bench_allreduce_free(bench_t *bench);
int bench_allreduce_init_test(bench_t *bench);
int bench_allreduce_free_test(bench_t *bench);
int bench_allreduce_printinfo(bench_t *bench);
double measure_allreduce_sync(bench_t *bench);

#endif /* ALLREDUCE_H */
