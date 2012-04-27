/*
 * alltoallw.c: Benchmark functions for Alltoallw.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdlib.h>

#include <mpi.h>

#include "alltoallw.h"
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
static MPI_Datatype *types = NULL;
static int sbufsize;
static int rbufsize;

/* bench_alltoallw_init: */
int bench_alltoallw_init(colltest_params_t *params)
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

/* bench_alltoallw_free: */
int bench_alltoallw_free()
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

/* bench_alltoallw_printinfo: */
int bench_alltoallw_printinfo()
{
    printf("* Alltoallw\n"
           "  proto: MPI_Alltoallw(sbuf, sendcounts, sdispls, sendtypes, \n"
           "                       rbuf, recvcounts, rdispls, recvtypes, \n"
           "                       comm)\n"
           "  For each element: sendtypes[i] = recvtypes[i] = MPI_BYTE\n"
           "  For each element: sendcounts[i] = recvcounts[i] = count\n");
    return MPIPERF_SUCCESS;
}

/* measure_alltoallw_sync: */
int measure_alltoallw_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Alltoallw(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                       types, mempool_alloc(rbufpool, rbufsize), recvcounts,
                       rdispls, types, params->comm);
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

