/*
 * exscan.h: Benchmark functions for Exscan.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef EXSCAN_H
#define EXSCAN_H

#include "bench_coll.h"

int bench_exscan_init(colltest_params_t *params);
int bench_exscan_free();
int bench_exscan_printinfo();
int measure_exscan_sync(colltest_params_t *params, double *time);

#endif /* EXSCAN_H */
