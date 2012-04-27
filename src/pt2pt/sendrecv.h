/*
 * sendrecv.h: Benchmark functions for sendrecv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SENDRECV_H
#define SENDRECV_H

#include "bench_pt2pt.h"

int bench_sendrecv_init(pt2pttest_params_t *params);
int bench_sendrecv_free();
int bench_sendrecv_printinfo();
int measure_sendrecv_sync(pt2pttest_params_t *params, double *time);

#endif /* SENDRECV_H */
