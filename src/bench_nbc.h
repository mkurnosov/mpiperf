/*
 * bench_nbc.h: Functions for benchmarking MPI non-blocking collective routines.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#ifndef BENCH_NBC_H
#define BENCH_NBC_H

#include <mpi.h>

#include "stat.h"

/* Test parameters */
typedef struct {
	MPI_Comm comm;
	int nprocs;
	int count;
	double comptime;
} nbctest_params_t;

/* Test results */
typedef struct {
	double inittime;
	double waittime;
	double realcomptime;
	double totaltime;
	int ntests;
} nbctest_result_t;

typedef int (*nbcbench_init_ptr_t)(nbctest_params_t *params);
typedef void (*nbcbench_free_ptr_t)();
typedef void (*nbcbench_printinfo_ptr_t)();
typedef int (*nbcbench_collop_blocking_ptr_t)(nbctest_params_t *params,
		                                      nbctest_result_t *result);
typedef int (*nbcbench_collop_overlap_ptr_t)(nbctest_params_t *params,
		                                     nbctest_result_t *result);

/* Benchmark */
typedef struct {
    char *name;                                 /* Benchmark name */
    nbcbench_init_ptr_t init;                   /* Called before measures */
    nbcbench_free_ptr_t free;                   /* Called after measures */
    nbcbench_printinfo_ptr_t printinfo;
    nbcbench_collop_blocking_ptr_t blockingop;  /* Measures NBC in blocking mode */
    nbcbench_collop_overlap_ptr_t overlapop;    /* Measures NBC overlap */
} nbcbench_t;

extern nbcbench_t nbcbenchtab[];

void print_nbcbench_info();
nbcbench_t *lookup_nbcbench(const char *name);

int run_nbcbench(nbcbench_t *bench);
int run_nbcbench_overlap(nbcbench_t *bench, nbctest_params_t *params);
int run_nbcbench_overlap_test(nbcbench_t *bench,
	                          nbctest_params_t *params,
		                      double blockingtime,
		                      int *nruns, int *ncorrectruns,
		                      stat_sample_t *inittimestat,
		                      stat_sample_t *waittimestat,
		                      stat_sample_t *comptimestat,
		                      stat_sample_t *totaltimestat,
		                      stat_sample_t *overlapstat);

int nbcbench_measure_blocking_time(nbcbench_t *bench, nbctest_params_t *params,
		                           double *globaltime, double *localtime);

int nbcbench_simulate_computing(nbctest_params_t *params,
		                        MPI_Request *request, nbctest_result_t *result);

int report_write_nbcbench_header(nbcbench_t *bench);
int report_write_nbcbench_procstat_header(nbcbench_t *bench);

int report_write_nbcbench_overlap(nbcbench_t *bench, nbctest_params_t *params,
                                  int nruns, int ncorrectruns,
                                  double blockingtime,
                                  stat_sample_t *inittimestat,
                                  stat_sample_t *waittimestat,
                                  stat_sample_t *comptimestat,
                                  stat_sample_t *totaltimestat,
                                  stat_sample_t *overlapstat);

int report_write_nbcbench_procstat_overlap(nbcbench_t *bench,
		                                   nbctest_params_t *params,
                                           int nruns, int ncorrectruns,
                                           double blockingtime,
                                           stat_sample_t *inittimestat,
                                           stat_sample_t *waittimestat,
                                           stat_sample_t *comptimestat,
                                           stat_sample_t *totaltimestat,
                                           stat_sample_t *overlapstat);

int run_nbcbench_blocking(nbcbench_t *bench, nbctest_params_t *params);

int run_nbcbench_blocking_test(nbcbench_t *bench,
	                           nbctest_params_t *params,
		                       int *nruns, int *ncorrectruns,
		                       stat_sample_t *inittimestat,
		                       stat_sample_t *waittimestat,
		                       stat_sample_t *totaltimestat,
		                       stat_sample_t *inittimestat_local,
		                       stat_sample_t *waittimestat_local,
		                       stat_sample_t *totaltimestat_local);

int report_write_nbcbench_blocking(nbcbench_t *bench,
		                           nbctest_params_t *params,
                                   int nruns, int ncorrectruns,
                                   stat_sample_t *inittimestat,
                                   stat_sample_t *waittimestat,
                                   stat_sample_t *totaltimestat);

int report_write_nbcbench_procstat_blocking(nbcbench_t *bench,
		                                    nbctest_params_t *params,
                                            int nruns, int ncorrectruns,
                                            stat_sample_t *inittimestat,
                                            stat_sample_t *waittimestat,
                                            stat_sample_t *totaltimestat);

#endif /* BENCH_NBC_H */
