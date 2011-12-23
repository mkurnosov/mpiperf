/*
 * bench.h: Functions for benchmarking MPI routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef BENCH_H
#define BENCH_H

#include <mpi.h>

#include "stats.h"

#define IS_MEASURE_INVALID(time) ((MEASURE_TIME_INVVAL - (time)) < 1.0)
#define IS_MEASURE_STARTED_LATE(time) ((MEASURE_STARTED_LATE - (time)) < 1.0)
#define IS_MEASURE_TIME_TOOLONG(time) ((MEASURE_TIME_TOOLONG - (time)) < 1.0)

#define MEASURE_TIME_INVVAL 1E10
#define MEASURE_STARTED_LATE (MEASURE_TIME_INVVAL + 10)
#define MEASURE_TIME_TOOLONG (MEASURE_TIME_INVVAL + 20)

enum MPIFunctionType {
    MPIFUNC_TYPE_COLL = 1,
    MPIFUNC_TYPE_PT2PT = 2
};

enum ParameterType {
    PARAM_STEP_INC = 0,       /* param[i] = param[i - 1] + step */
    PARAM_STEP_MUL = 1        /* param[i] = param[i - 1] * step */
};

enum {
    TEST_EXIT_COND_NRUNS = 0,
    TEST_EXIT_COND_STDERR = 1
};

typedef struct bench bench_t;
typedef void (*printinfo_ptr_t)(bench_t *bench);
typedef int (*bench_init_ptr_t)(bench_t *bench);
typedef void (*bench_free_ptr_t)(bench_t *bench);
typedef int (*bench_init_test_ptr_t)(bench_t *bench);
typedef void (*bench_free_test_ptr_t)(bench_t *bench);
typedef double (*measure_ptr_t)(bench_t *bench);
typedef MPI_Comm (*getcomm_ptr_t)(bench_t *bench);
typedef int (*getcommsize_ptr_t)(bench_t *bench);
typedef int (*getrank_ptr_t)(bench_t *bench);
typedef int (*paramseq_getcurrent_ptr_t)(bench_t *bench);
typedef int (*paramseq_getnext_ptr_t)(bench_t *bench);
typedef int (*paramseq_getsize_ptr_t)(bench_t *bench);
typedef void (*paramseq_reset_ptr_t)(bench_t *bench);

/*
 * Benchmark
 */
struct bench {
    char *name;                       /* Benchmark name */
    int mpifunctype;                  /* Routine type: collective, pt2pt */

    printinfo_ptr_t printinfo;
    bench_init_ptr_t init;            /* Called before all tests */
    bench_free_ptr_t free;            /* Called after all tests */
    bench_init_test_ptr_t init_test;  /* Called before each test */
    bench_free_test_ptr_t free_test;  /* Called after each test */
    measure_ptr_t measure_sync;       /* Pointer to measure function */

    getcomm_ptr_t getcomm;            /* Returns current test's communicator */
    getcommsize_ptr_t getcommsize;
    getrank_ptr_t getrank;

    /* Sequence of parameter values */
    char *paramname;                  /* Parameter name: sendcount, commsize */
    paramseq_getnext_ptr_t paramseq_getcurrent;
    paramseq_getnext_ptr_t paramseq_getnext;
    paramseq_getsize_ptr_t paramseq_getsize;
    paramseq_reset_ptr_t paramseq_reset;
};

int bench_initialize(bench_t *bench);
int bench_finalize(bench_t *bench);
void print_benchmarks_info();
bench_t *lookup_bench(const char *name);
int is_pt2pt_benchmark(bench_t *bench);
double measure_bcast_double(MPI_Comm comm);

int run_collective_bench(bench_t *bench);
int run_collective_test_synctime(bench_t *bench, double **exectime,
		                         int *nmeasurements, int *ncorrect_measurements,
		                         stats_sample_t *procstat);
int run_collective_test_nosync(bench_t *bench, double *exectime,
		                       stats_sample_t *procstat, int *nmeasurements);

int run_pt2pt_bench(bench_t *bench);
int run_pt2pt_test_synctime(bench_t *bench, double **exectime, int *nmeasurements,
                            int *ncorrect_measurements);
MPI_Comm getcommworld();
int getcommworld_size(bench_t *bench);
int getcommworld_rank(bench_t *bench);

#endif /* BENCH_H */

