/*
 * alltoallv.c: Benchmark functions for Alltoallv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdlib.h>

#include <mpi.h>

#include "alltoallv.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
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

/* bench_alltoallv_init: */
int bench_alltoallv_init(colltest_params_t *params)
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

/* bench_alltoallv_free: */
int bench_alltoallv_free()
{
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

/* bench_alltoallv_printinfo: */
int bench_alltoallv_printinfo()
{
    printf("* Alltoallv\n"
           "  proto: MPI_Alltoallv(sbuf, sendcounts, sdispls, MPI_BYTE, \n"
           "                       rbuf, recvcounts, rdispls, MPI_BYTE, \n"
           "                       comm)\n"
           "  For each element: sendcounts[i] = recvcounts[i] = count\n");
    return MPIPERF_SUCCESS;
}

/* measure_alltoallv_sync: */
int measure_alltoallv_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Alltoallv(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                       MPI_BYTE, mempool_alloc(rbufpool, rbufsize), recvcounts,
                       rdispls, MPI_BYTE, params->comm);
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

