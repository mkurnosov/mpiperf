/*
 * iscan.h: Benchmark functions for Iscan.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ISCAN_H
#define ISCAN_H

#include "bench_nbc.h"

int bench_iscan_init(nbctest_params_t *params);
int bench_iscan_free();
int bench_iscan_printinfo();
int measure_iscan_blocking(nbctest_params_t *params,
                           nbctest_result_t *result);
int measure_iscan_overlap(nbctest_params_t *params,
                          nbctest_result_t *result);

#endif /* ISCAN_H */
