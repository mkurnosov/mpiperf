/*
 * iscatterv.h: Benchmark functions for Iscatterv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef ISCATTERV_H
#define ISCATTERV_H

#include "bench_nbc.h"

int bench_iscatterv_init(nbctest_params_t *params);
int bench_iscatterv_free();
int bench_iscatterv_printinfo();
int measure_iscatterv_blocking(nbctest_params_t *params,
		                       nbctest_result_t *result);
int measure_iscatterv_overlap(nbctest_params_t *params,
		                      nbctest_result_t *result);

#endif /* ISCATTERV_H */
