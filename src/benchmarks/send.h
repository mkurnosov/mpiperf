/*
 * send.h: Benchmark functions for send.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SEND_H
#define SEND_H

#include "bench.h"

int bench_send_init(bench_t *bench);
int bench_send_free(bench_t *bench);
int bench_send_init_test(bench_t *bench);
int bench_send_free_test(bench_t *bench);
int bench_send_printinfo(bench_t *bench);
double measure_send_sync(bench_t *bench);
MPI_Comm bench_send_getcomm(bench_t *bench);
int bench_send_getcommsize(bench_t *bench);
int bench_send_getcommrank(bench_t *bench);

#endif /* SEND_H */
