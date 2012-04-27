/*
 * waitpattern.c: Benchmark functions for wait routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef WAITPATTERN_H
#define WAITPATTERN_H

#include "bench_coll.h"

int bench_waitpattern_init(colltest_params_t *params);
int bench_waitpattern_free();

int bench_waitpatternup_printinfo();
int measure_waitpatternup_sync(colltest_params_t *params, double *time);

int bench_waitpatterndown_printinfo();
int measure_waitpatterndown_sync(colltest_params_t *params, double *time);

int bench_waitpatternnull_printinfo();
int measure_waitpatternnull_sync(colltest_params_t *params, double *time);

#endif /* WAITPATTERN_H */
