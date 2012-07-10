/*
 * scan.h: Benchmark functions for Scan.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SCAN_H
#define SCAN_H

#include "bench_coll.h"

int bench_scan_init(colltest_params_t *params);
int bench_scan_free();
int bench_scan_printinfo();
int measure_scan_sync(colltest_params_t *params, double *time);

#endif /* SCAN_H */
