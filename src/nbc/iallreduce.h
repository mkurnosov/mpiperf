/*
 * iallreduce.h: Benchmark functions for Iallreduce.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IALLREDUCE_H
#define IALLREDUCE_H

#include "bench_nbc.h"

int bench_iallreduce_init(nbctest_params_t *params);
int bench_iallreduce_free();
int bench_iallreduce_printinfo();
int measure_iallreduce_blocking(nbctest_params_t *params,
                                nbctest_result_t *result);
int measure_iallreduce_overlap(nbctest_params_t *params,
                               nbctest_result_t *result);

#endif /* IALLREDUCE_H */
