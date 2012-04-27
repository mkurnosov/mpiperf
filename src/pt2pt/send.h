/*
 * send.h: Benchmark functions for send.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SEND_H
#define SEND_H

#include "bench_pt2pt.h"

int bench_send_init(pt2pttest_params_t *params);
int bench_send_free();
int bench_send_printinfo();
int measure_send_sync(pt2pttest_params_t *params, double *time);

#endif /* SEND_H */
