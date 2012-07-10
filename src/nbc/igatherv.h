/*
 * igatherv.h: Benchmark functions for Igatherv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IGATHERV_H
#define IGATHERV_H

#include "bench_nbc.h"

int bench_igatherv_init(nbctest_params_t *params);
int bench_igatherv_free();
int bench_igatherv_printinfo();
int measure_igatherv_blocking(nbctest_params_t *params,
                              nbctest_result_t *result);
int measure_igatherv_overlap(nbctest_params_t *params,
                             nbctest_result_t *result);

#endif /* IGATHERV_H */
