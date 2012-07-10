/*
 * ialltoallw.c: Benchmark functions for Ialltoallw.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "ialltoallw.h"
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
static MPI_Datatype *types = NULL;
static int sbufsize;
static int rbufsize;

int bench_ialltoallw_init(nbctest_params_t *params)
{
    int i;

    types = malloc(sizeof(*types) * params->nprocs);
    if (types == NULL) {
        goto errhandler;
    }

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
        types[i] = MPI_BYTE;
    }
    for (i = 0; i < params->nprocs; i++) {
        rdispls[i] = i * params->count * sizeof(char);
        sdispls[i] = i * params->count * sizeof(char);
    }
    sbufsize = params->count * sizeof(char) * params->nprocs;
    rbufsize = params->count * sizeof(char) * params->nprocs;

    return MPIPERF_SUCCESS;

errhandler:
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    free(types);
    mempool_free(sbufpool);
    mempool_free(rbufpool);

    return MPIPERF_FAILURE;
}

int bench_ialltoallw_free()
{
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    free(types);
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

int bench_ialltoallw_printinfo()
{
    printf("* Ialltoallw\n"
           "  proto: MPI_Ialltoallw(sbuf, sendcounts, sdispls, sendtypes, \n"
           "                        rbuf, recvcounts, rdispls, recvtypes, \n"
           "                        comm)\n"
           "  For each element: sendtypes[i] = recvtypes[i] = MPI_BYTE\n"
           "  For each element: sendcounts[i] = recvcounts[i] = count\n");
    return MPIPERF_SUCCESS;
}

int measure_ialltoallw_blocking(nbctest_params_t *params,
                                nbctest_result_t *result)
{
#ifdef HAVE_NBC
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPI_Ialltoallw(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                        types, mempool_alloc(rbufpool, rbufsize), recvcounts,
                        rdispls, types, params->comm, &req);
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

int measure_ialltoallw_overlap(nbctest_params_t *params,
                               nbctest_result_t *result)

{
#ifdef HAVE_NBC
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPI_Ialltoallw(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                        types, mempool_alloc(rbufpool, rbufsize), recvcounts,
                        rdispls, types, params->comm, &req);
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
