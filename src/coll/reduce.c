/*
 * reduce.c: Benchmark functions for Reduce.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "reduce.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;
static int root = 0;

/* bench_reduce_init: */
int bench_reduce_init(colltest_params_t *params)
{
    int rank;

    rbufpool = NULL;
    rbufsize = 0;

    sbufpool = mempool_create(params->count * sizeof(double), mpiperf_isflushcache);
    if (sbufpool == NULL) {
        return MPIPERF_FAILURE;
    }
    sbufsize = params->count * sizeof(double);

    MPI_Comm_rank(params->comm, &rank);
    if (rank == root) {
        rbufpool = mempool_create(params->count * sizeof(double),
                                  mpiperf_isflushcache);
        if (rbufpool == NULL) {
            mempool_free(sbufpool);
            return MPIPERF_FAILURE;
        }
        rbufsize = params->count * sizeof(double);
    }
    return MPIPERF_SUCCESS;
}

/* bench_reduce_free: */
int bench_reduce_free(collbench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_printinfo: */
int bench_reduce_printinfo()
{
    printf("* Reduce\n"
           "  proto: MPI_Reduce(sbuf, rbuf, count, MPI_DOUBLE, \n"
           "                    MPI_SUM, %d, comm)\n", root);
    return MPIPERF_SUCCESS;
}

/* measure_reduce_sync: */
int measure_reduce_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Reduce(mempool_alloc(sbufpool, sbufsize),
                    mempool_alloc(rbufpool, sbufsize), params->count, MPI_DOUBLE,
                    MPI_SUM, root, params->comm);
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

