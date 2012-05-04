/*
 * iexscan.h: Benchmark functions for Iexscan.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IEXSCAN_H
#define IEXSCAN_H

#include "bench_nbc.h"

int bench_iexscan_init(nbctest_params_t *params);
int bench_iexscan_free();
int bench_iexscan_printinfo();
int measure_iexscan_blocking(nbctest_params_t *params,
 		                     nbctest_result_t *result);
int measure_iexscan_overlap(nbctest_params_t *params,
		                    nbctest_result_t *result);

#endif /* IEXSCAN_H */
