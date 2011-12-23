/*
 * scan.h: Benchmark functions for Scan.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SCAN_H
#define SCAN_H

#include "bench.h"

int bench_scan_init(bench_t *bench);
int bench_scan_free(bench_t *bench);
int bench_scan_init_test(bench_t *bench);
int bench_scan_free_test(bench_t *bench);
int bench_scan_printinfo(bench_t *bench);
double measure_scan_sync(bench_t *bench);

#endif /* SCAN_H */
