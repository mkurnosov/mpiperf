/*
 * waitpattern.c: Benchmark functions for wait routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef WAITPATTERN_H
#define WAITPATTERN_H

#include "bench.h"

int bench_waitpattern_init(bench_t *bench);
int bench_waitpattern_free(bench_t *bench);
int bench_waitpattern_init_test(bench_t *bench);
int bench_waitpattern_free_test(bench_t *bench);
int bench_waitpatternup_printinfo(bench_t *bench);
double measure_waitpatternup_sync(bench_t *bench);
int bench_waitpatterndown_printinfo(bench_t *bench);
double measure_waitpatterndown_sync(bench_t *bench);
int bench_waitpatternnull_printinfo(bench_t *bench);
double measure_waitpatternnull_sync(bench_t *bench);

#endif /* WAITPATTERN_H */
