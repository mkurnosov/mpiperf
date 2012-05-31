/*
 * ialltoallv.c: Benchmark functions for Ialltoallv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "ialltoallv.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *sendcounts = NULL;
static int *recvcounts = NULL;
static int *sdispls = NULL;
static int *rdispls = NULL;
static int sbufsize;
static int rbufsize;

int bench_ialltoallv_init(nbctest_params_t *params)
{
    int i;

    recvcounts = malloc(sizeof(*recvcounts) * params->nprocs);
    rdispls = malloc(sizeof(*rdispls) * params->nprocs);
    if (recvcounts == NULL || rdispls == NULL) {
        goto errhandler;
    }

    sendcounts = malloc(sizeof(*sendcounts) * params->nprocs);
    sdispls = malloc(sizeof(*sdispls) * params->nprocs);
    if (sendcounts == NULL || sdispls == NULL) {
        goto errhandler;
    }

    sbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        goto errhandler;
    }

    for (i = 0; i < params->nprocs; i++) {
        recvcounts[i] = params->count;
        sendcounts[i] = params->count;
    }
    rdispls[0] = sdispls[0] = 0;
    for (i = 1; i < params->nprocs; i++) {
        rdispls[i] = rdispls[i - 1] + recvcounts[i - 1];
        sdispls[i] = sdispls[i - 1] + sendcounts[i - 1];
    }
    sbufsize = params->count * sizeof(char) * params->nprocs;
    rbufsize = params->count * sizeof(char) * params->nprocs;

    return MPIPERF_SUCCESS;

errhandler:
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    mempool_free(sbufpool);
    mempool_free(rbufpool);

    return MPIPERF_FAILURE;
}

int bench_ialltoallv_free()
{
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

int bench_ialltoallv_printinfo()
{
    printf("* Ialltoallv\n"
           "  proto: MPI_Ialltoallv(sbuf, sendcounts, sdispls, MPI_BYTE, \n"
           "                        rbuf, recvcounts, rdispls, MPI_BYTE, \n"
           "                        comm)\n"
           "  For each element: sendcounts[i] = recvcounts[i] = count\n");
    return MPIPERF_SUCCESS;
}

int measure_ialltoallv_blocking(nbctest_params_t *params,
                                nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Ialltoallv(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                         MPI_BYTE, mempool_alloc(rbufpool, rbufsize), recvcounts,
                         rdispls, MPI_BYTE, params->comm, &req);
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

int measure_ialltoallv_overlap(nbctest_params_t *params,
                               nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Ialltoallv(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                         MPI_BYTE, mempool_alloc(rbufpool, rbufsize), recvcounts,
                         rdispls, MPI_BYTE, params->comm, &req);
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
