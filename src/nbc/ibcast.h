/*
 * ibcast.h: Benchmark functions for Ibcast.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IBCAST_H
#define IBCAST_H

#include "bench_nbc.h"

int bench_ibcast_init(nbctest_params_t *params);
int bench_ibcast_free();
int bench_ibcast_printinfo();
int measure_ibcast_blocking(nbctest_params_t *params,
                            nbctest_result_t *result);
int measure_ibcast_overlap(nbctest_params_t *params,
                           nbctest_result_t *result);

#endif /* IBCAST_H */
