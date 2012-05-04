/*
 * iscatter.h: Benchmark functions for Iscatter.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ISCATTER_H
#define ISCATTER_H

#include "bench_nbc.h"

int bench_iscatter_init(nbctest_params_t *params);
int bench_iscatter_free();
int bench_iscatter_printinfo();
int measure_iscatter_blocking(nbctest_params_t *params,
                              nbctest_result_t *result);
int measure_iscatter_overlap(nbctest_params_t *params,
                             nbctest_result_t *result);

#endif /* ISCATTER_H */
