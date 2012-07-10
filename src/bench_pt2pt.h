/*
 * bench_pt2pt.h: Functions for benchmarking MPI point-to-point routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef BENCH_PT2PT_H
#define BENCH_PT2PT_H

#include <mpi.h>

#include "stat.h"

/* Test parameters */
typedef struct {
    MPI_Comm comm;
    int nprocs;
    int count;
} pt2pttest_params_t;

typedef int (*pt2ptbench_init_ptr_t)(pt2pttest_params_t *params);
typedef void (*pt2ptbench_free_ptr_t)();
typedef void (*pt2ptbench_printinfo_ptr_t)();
typedef int (*pt2ptbench_op_ptr_t)(pt2pttest_params_t *params, double *time);

/* Benchmark */
typedef struct {
    char *name;                            /* Benchmark name */
    pt2ptbench_init_ptr_t init;            /* Called before measures */
    pt2ptbench_free_ptr_t free;            /* Called after measures */
    pt2ptbench_printinfo_ptr_t printinfo;
    pt2ptbench_op_ptr_t op;                /* Pointer to measure function */
} pt2ptbench_t;


void print_pt2ptbench_info();
pt2ptbench_t *lookup_pt2ptbench(const char *name);

int run_pt2ptbench(pt2ptbench_t *bench);
int run_pt2ptbench_test(pt2ptbench_t *bench, pt2pttest_params_t *params);
int run_pt2ptbench_test_synctime(pt2ptbench_t *bench,
                                 pt2pttest_params_t *params, double **exectime,
                                 int *nmeasurements, int *ncorrect_measurements);

int report_write_pt2ptbench_header(pt2ptbench_t *bench);
int report_write_pt2pttest_synctime(pt2ptbench_t *bench,
                                    pt2pttest_params_t *params,
                                    double *exectime, int nruns,
                                    int ncorrectruns);
#endif /* BENCH_PT2PT_H */
