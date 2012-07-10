/*
 * bcast.h: Benchmark functions for Bcast.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef BCAST_H
#define BCAST_H

#include "bench_coll.h"

int bench_bcast_init(colltest_params_t *params);
int bench_bcast_free();
int bench_bcast_printinfo();
int measure_bcast_sync(colltest_params_t *params, double *time);

#endif /* BCAST_H */
