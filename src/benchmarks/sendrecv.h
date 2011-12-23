/*
 * sendrecv.h: Benchmark functions for sendrecv.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef SENDRECV_H
#define SENDRECV_H

#include "bench.h"

int bench_sendrecv_init(bench_t *bench);
int bench_sendrecv_free(bench_t *bench);
int bench_sendrecv_init_test(bench_t *bench);
int bench_sendrecv_free_test(bench_t *bench);
int bench_sendrecv_printinfo(bench_t *bench);
double measure_sendrecv_sync(bench_t *bench);
MPI_Comm bench_sendrecv_getcomm(bench_t *bench);
int bench_sendrecv_getcommsize(bench_t *bench);
int bench_sendrecv_getcommrank(bench_t *bench);

#endif /* SENDRECV_H */
