/*
 * ireduce_scatter.c: Benchmark functions for Ireduce_scatter.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "ireduce_scatter.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *recvcounts = NULL;
static int sbufsize;
static int rbufsize;

int bench_ireduce_scatter_init(nbctest_params_t *params)
{
    int i, rank;

    recvcounts = malloc(sizeof(*recvcounts) * params->nprocs);
    if (recvcounts == NULL) {
        return MPIPERF_FAILURE;
    }

    sbufpool = mempool_create(params->count * sizeof(double), mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * sizeof(double) * params->nprocs,
                              mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }

    for (i = 0; i < params->nprocs; i++) {
        recvcounts[i] = params->count / params->nprocs;
    }
    for (i = 0; i < params->count % params->nprocs; i++) {
        recvcounts[i * params->nprocs / (params->count % params->nprocs)]++;
    }
    MPI_Comm_rank(params->comm, &rank);
    sbufsize = params->count * sizeof(double);
    rbufsize = recvcounts[rank] * sizeof(double);

    return MPIPERF_SUCCESS;
}

int bench_ireduce_scatter_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(recvcounts);
    return MPIPERF_SUCCESS;
}

int bench_ireduce_scatter_printinfo()
{
    printf("* Ireduce_scatter\n"
           "  proto: MPI_Ireduce_scatter(sbuf, rbuf, recvcounts, MPI_DOUBLE, \n"
           "                             MPI_SUM, comm)\n"
           "  Send buffer is divided onto equal parts and scattered among processes\n");
    return MPIPERF_SUCCESS;
}

int measure_ireduce_scatter_blocking(nbctest_params_t *params,
                                     nbctest_result_t *result)
{
#ifdef HAVE_NBC
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPI_Ireduce_scatter(mempool_alloc(sbufpool, sbufsize),
                             mempool_alloc(rbufpool, rbufsize), recvcounts,
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

int measure_ireduce_scatter_overlap(nbctest_params_t *params,
                                    nbctest_result_t *result)

{
#ifdef HAVE_NBC
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPI_Ireduce_scatter(mempool_alloc(sbufpool, sbufsize),
                             mempool_alloc(rbufpool, rbufsize), recvcounts,
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
#endif
    return MEASURE_FAILURE;
}
