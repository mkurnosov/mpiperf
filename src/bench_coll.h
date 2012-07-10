/*
 * bench_coll.h: Functions for benchmarking MPI collective routines.
 *
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#ifndef BENCH_COLL_H
#define BENCH_COLL_H

#include <mpi.h>

#include "stat.h"

/* Test parameters */
typedef struct {
    MPI_Comm comm;
    int nprocs;
    int count;
} colltest_params_t;

typedef int (*collbench_init_ptr_t)(colltest_params_t *params);
typedef void (*collbench_free_ptr_t)();
typedef void (*collbench_printinfo_ptr_t)();
typedef int (*collbench_op_ptr_t)(colltest_params_t *params, double *time);

/* Benchmark */
typedef struct {
    char *name;                           /* Benchmark name */
    collbench_init_ptr_t init;            /* Called before measures */
    collbench_free_ptr_t free;            /* Called after measures */
    collbench_printinfo_ptr_t printinfo;
    collbench_op_ptr_t collop;            /* Pointer to measure function */
} collbench_t;

extern collbench_t collbenchtab[];

void print_collbench_info();
collbench_t *lookup_collbench(const char *name);

int run_collbench(collbench_t *bench);
int run_collbench_test(collbench_t *bench, colltest_params_t *params);
int run_collbench_test_synctime(collbench_t *bench, colltest_params_t *params,
                                double **exectime, int *nmeasurements,
                                int *ncorrect_measurements,
                                stat_sample_t *procstat);
int run_collbench_test_nosync(collbench_t *bench, colltest_params_t *params,
                              double *exectime, int *nmeasurements,
                              stat_sample_t *procstat);

int report_write_collbench_header(collbench_t *bench);
int report_write_colltest_synctime(collbench_t *bench, colltest_params_t *params,
                                   double *exectime, int nruns, int ncorrectruns);
int report_write_colltest_nosync(collbench_t *bench, colltest_params_t *params,
                                 double exectime_local, int nruns);

int report_write_collbench_procstat_header(collbench_t *bench);
int report_write_collbench_procstat_synctime(collbench_t *bench,
                                             colltest_params_t *params,
                                             stat_sample_t *procstat);
int report_write_collbench_procstat_nosync(collbench_t *bench,
                                           colltest_params_t *params,
                                           stat_sample_t *procstat);

#endif /* BENCH_COLL_H */
