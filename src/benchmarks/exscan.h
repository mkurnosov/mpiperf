/*
 * exscan.h: Benchmark functions for Exscan.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef EXSCAN_H
#define EXSCAN_H

#include "bench.h"

int bench_exscan_init(bench_t *bench);
int bench_exscan_free(bench_t *bench);
int bench_exscan_init_test(bench_t *bench);
int bench_exscan_free_test(bench_t *bench);
int bench_exscan_printinfo(bench_t *bench);
double measure_exscan_sync(bench_t *bench);

#endif /* EXSCAN_H */
