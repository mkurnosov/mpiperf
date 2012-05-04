/*
 * reduce_scatter_block.c: Benchmark functions for Reduce_scatter_block.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "reduce_scatter_block.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;

/* bench_reduce_scatter_block_init: */
int bench_reduce_scatter_block_init(colltest_params_t *params)
{
    sbufpool = mempool_create(params->count * sizeof(double) * params->nprocs,
                              mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * sizeof(double), mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    sbufsize = params->count * sizeof(double) * params->nprocs;
    rbufsize = params->count * sizeof(double);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_block_free: */
int bench_reduce_scatter_block_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_block_printinfo: */
int bench_reduce_scatter_block_printinfo()
{
    printf("* Reduce_scatter_block\n"
           "  proto: MPI_Reduce_scatter_block(sbuf, rbuf, count, MPI_DOUBLE, \n"
           "                                  MPI_SUM, comm)\n");
    return MPIPERF_SUCCESS;
}

/* measure_reduce_scatter_block_sync: */
int measure_reduce_scatter_block_sync(colltest_params_t *params, double *time)
{
#ifdef MPICH2
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Reduce_scatter_block(mempool_alloc(sbufpool, sbufsize),
                                  mempool_alloc(rbufpool, rbufsize), params->count,
                                  MPI_DOUBLE, MPI_SUM, params->comm);
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        *time = endtime - starttime;
        return MEASURE_SUCCESS;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
#endif
    return MEASURE_FAILURE;
}

