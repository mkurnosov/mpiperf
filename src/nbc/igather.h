/*
 * igather.h: Benchmark functions for Igather.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef IGATHER_H
#define IGATHER_H

#include "bench_nbc.h"

int bench_igather_init(nbctest_params_t *params);
int bench_igather_free();
int bench_igather_printinfo();
int measure_igather_blocking(nbctest_params_t *params,
		                     nbctest_result_t *result);
int measure_igather_overlap(nbctest_params_t *params,
		                    nbctest_result_t *result);

#endif /* IGATHER_H */
