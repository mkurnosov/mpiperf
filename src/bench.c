/*
 * bench.c: Functions for benchmarking MPI routines.
 *
 * Copyright (C) 2010-2011 Mikhail Kurnosov
 */

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <mpi.h>

#include "bench.h"
#include "mpiperf.h"
#include "seq.h"
#include "stats.h"
#include "timeslot.h"
#include "report.h"
#include "logger.h"
#include "util.h"
#include "hpctimer.h"
#include "benchmarks/barrier.h"
#include "benchmarks/bcast.h"
#include "benchmarks/gather.h"
#include "benchmarks/gatherv.h"
#include "benchmarks/scatter.h"
#include "benchmarks/scatterv.h"
#include "benchmarks/allgather.h"
#include "benchmarks/allgatherv.h"
#include "benchmarks/alltoall.h"
#include "benchmarks/alltoallv.h"
#include "benchmarks/alltoallw.h"
#include "benchmarks/reduce.h"
#include "benchmarks/allreduce.h"
#include "benchmarks/reduce_scatter_block.h"
#include "benchmarks/reduce_scatter.h"
#include "benchmarks/scan.h"
#include "benchmarks/exscan.h"
#include "benchmarks/waitpattern.h"
#include "benchmarks/send.h"
#include "benchmarks/sendrecv.h"

#include "benchtab.h"

/* bench_initialize: Initializes benchmark before all tests. */
int bench_initialize(bench_t *bench)
{
    if (bench->init) {
        return bench->init(bench);
    }
    return MPIPERF_SUCCESS;
}

/* bench_finalize: */
int bench_finalize(bench_t *bench)
{
    if (bench) {
        if (bench->free) {
            bench->free(bench);
        }
    }
    return MPIPERF_SUCCESS;
}

/* lookup_bench: Returns pointer to benchmark by name or NULL on error. */
bench_t *lookup_bench(const char *name)
{
    int i;

    for (i = 0; i < NELEMS(benchtab); i++) {
        if (strcasecmp(benchtab[i].name, name) == 0) {
            return &benchtab[i];
        }
    }
    return NULL;
}

/* print_benchmarks_info: Prints information about each benchmark. */
void print_benchmarks_info()
{
    int i;

    for (i = 0; i < NELEMS(benchtab); i++) {
        printf("\n=== Benchmark %d ===\n", i + 1);
        if (benchtab[i].printinfo) {
            benchtab[i].printinfo(&benchtab[i]);
        }
    }
}

/*
 * measure_bcast_double: Measures MPI_Bcast function time
 *                       for sending one element of type double.
 *                       This value is used for determining start time
 *                       of the first timeslot.
 */
double measure_bcast_double(MPI_Comm comm)
{
    double buf, totaltime = 0.0, optime, maxtime = 0.0;
    int i, nreps = 3;

    /* Warmup call */
    MPI_Bcast(&buf, 1, MPI_DOUBLE, mpiperf_master_rank, comm);
    /* Measures */
    for (i = 0; i < nreps; i++) {
        MPI_Barrier(comm);
        optime = hpctimer_wtime();
        MPI_Bcast(&buf, 1, MPI_DOUBLE, mpiperf_master_rank, comm);
        optime = hpctimer_wtime() - optime;
        MPI_Reduce(&optime, &maxtime, 1, MPI_DOUBLE, MPI_MAX,
                   mpiperf_master_rank, comm);
        totaltime = stats_fmax2(totaltime, maxtime);
    }
    return totaltime;
}

/* is_pt2pt_benchmark: */
int is_pt2pt_benchmark(bench_t *bench)
{
    return bench->mpifunctype == MPIFUNC_TYPE_PT2PT;
}

/* getcommworld: */
MPI_Comm getcommworld(bench_t *bench)
{
    return MPI_COMM_WORLD;
}

/* getcommworld_size: */
int getcommworld_size(bench_t *bench)
{
    return mpiperf_commsize;
}

/* getcommworld_rank: */
int getcommworld_rank(bench_t *bench)
{
    return mpiperf_rank;
}

