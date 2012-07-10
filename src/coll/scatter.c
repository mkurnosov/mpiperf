/*
 * scatter.c: Benchmark functions for Scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "scatter.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;
static int root = 0;

/* bench_scatter_init: */
int bench_scatter_init(colltest_params_t *params)
{
    int rank;

    sbufpool = NULL;
    sbufsize = 0;

    MPI_Comm_rank(params->comm, &rank);
    if (rank == root) {
        sbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
        if (sbufpool == NULL) {
            return MPIPERF_FAILURE;
        }
        sbufsize = params->count * sizeof(char);
    }
    rbufpool = mempool_create(params->count, mpiperf_isflushcache);
    if (rbufpool == NULL) {
        mempool_free(sbufpool);
        return MPIPERF_FAILURE;
    }
    rbufsize = params->count * sizeof(char);
    return MPIPERF_SUCCESS;
}

/* bench_scatter_free: */
int bench_scatter_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

/* bench_scatter_printinfo: */
int bench_scatter_printinfo(collbench_t *bench)
{
    printf("* Scatter\n"
           "  proto: MPI_Scatter(sbuf, count, MPI_BYTE, \n"
           "                     rbuf, count, MPI_BYTE, %d, comm)\n",
           root);
    return MPIPERF_SUCCESS;
}

/* measure_scatter_sync: */
int measure_scatter_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Scatter(mempool_alloc(sbufpool, sbufsize), params->count, MPI_BYTE,
                     mempool_alloc(rbufpool, rbufsize), params->count, MPI_BYTE,
                     root, params->comm);
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        *time = endtime - starttime;
        return MEASURE_SUCCESS;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_FAILURE;
}

