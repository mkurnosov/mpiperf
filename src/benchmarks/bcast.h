/*
 * bcast.h: Benchmark functions for Bcast.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef BCAST_H
#define BCAST_H

#include "bench.h"

int bench_bcast_init(bench_t *bench);
int bench_bcast_free(bench_t *bench);
int bench_bcast_init_test(bench_t *bench);
int bench_bcast_free_test(bench_t *bench);
int bench_bcast_printinfo(bench_t *bench);
double measure_bcast_sync(bench_t *bench);

#endif /* BCAST_H */
