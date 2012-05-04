/*
 * ireduce_scatter_block.c: Benchmark functions for Ireduce_scatter_block.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "ireduce_scatter_block.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;

int bench_ireduce_scatter_block_init(nbctest_params_t *params)
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

int bench_ireduce_scatter_block_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

int bench_ireduce_scatter_block_printinfo()
{
    printf("* Ireduce_scatter_block\n"
           "  proto: MPI_Ireduce_scatter_block(sbuf, rbuf, count, MPI_DOUBLE, \n"
           "                                   MPI_SUM, comm)\n");
    return MPIPERF_SUCCESS;
}

int measure_ireduce_scatter_block_blocking(nbctest_params_t *params,
                                           nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Ireduce_scatter_block(mempool_alloc(sbufpool, sbufsize),
                                    mempool_alloc(rbufpool, rbufsize), params->count,
                                    MPI_DOUBLE, MPI_SUM, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        result->totaltime = endtime - starttime;
        return MEASURE_SUCCESS;
    }
#endif
    return MEASURE_FAILURE;
}

int measure_ireduce_scatter_block_overlap(nbctest_params_t *params,
                                          nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Ireduce_scatter_block(mempool_alloc(sbufpool, sbufsize),
                                    mempool_alloc(rbufpool, rbufsize), params->count,
                                    MPI_DOUBLE, MPI_SUM, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    nbcbench_simulate_computing(params, &req, result);
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        result->totaltime = endtime - starttime;
        return MEASURE_SUCCESS;
    }
    return MEASURE_FAILURE;
#endif
}
