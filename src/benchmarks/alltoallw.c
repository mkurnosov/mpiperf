/*
 * alltoallw.c: Benchmark functions for Alltoallw.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "alltoallw.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *sendcounts = NULL;
static int *recvcounts = NULL;
static int *sdispls = NULL;
static int *rdispls = NULL;
static MPI_Datatype *types = NULL;
static int count;
static int sbufsize;
static int rbufsize;
static int commsize;
static MPI_Comm comm;

/* bench_alltoallw_init: */
int bench_alltoallw_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

    commsize = bench->getcommsize(bench);
    comm = bench->getcomm(bench);

    min = (mpiperf_param_min == -1) ? 1 : mpiperf_param_min;
    max = (mpiperf_param_max == -1) ? 1 << 18 : mpiperf_param_max;

    if (mpiperf_param_step_type == PARAM_STEP_INC) {
        steptype = SEQ_STEP_INC;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 0) ?
               64 : mpiperf_param_step;
    } else if (mpiperf_param_step_type == PARAM_STEP_MUL) {
        steptype = SEQ_STEP_MUL;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 1) ?
               2 : mpiperf_param_step;
    }
    if (intseq_initialize(bench, min, max, step, steptype) == MPIPERF_FAILURE)
        return MPIPERF_FAILURE;

    types = malloc(sizeof(*types) * bench->getcommsize(bench));
    if (types == NULL) {
        goto errhandler;
    }

    recvcounts = malloc(sizeof(*recvcounts) * bench->getcommsize(bench));
    rdispls = malloc(sizeof(*rdispls) * bench->getcommsize(bench));
    if (recvcounts == NULL || rdispls == NULL) {
        goto errhandler;
    }

    sendcounts = malloc(sizeof(*sendcounts) * bench->getcommsize(bench));
    sdispls = malloc(sizeof(*sdispls) * bench->getcommsize(bench));
    if (sendcounts == NULL || sdispls == NULL) {
        goto errhandler;
    }

    sbufpool = mempool_create(max * bench->getcommsize(bench),
                              mpiperf_isflushcache);
    rbufpool = mempool_create(max * bench->getcommsize(bench),
                              mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        goto errhandler;
    }
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
int bench_alltoallw_free(bench_t *bench)
{
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    free(types);
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_alltoallw_init_test: */
int bench_alltoallw_init_test(bench_t *bench)
{
    int i;

    count = bench->paramseq_getcurrent(bench);
    for (i = 0; i < commsize; i++) {
        recvcounts[i] = count;
        sendcounts[i] = count;
        types[i] = MPI_BYTE;
    }

    for (i = 0; i < commsize; i++) {
        rdispls[i] = i * count * sizeof(char);
        sdispls[i] = i * count * sizeof(char);
    }

    sbufsize = count * sizeof(char) * commsize;
    rbufsize = count * sizeof(char) * commsize;
    return MPIPERF_SUCCESS;
}

/* bench_alltoallw_free_test: */
int bench_alltoallw_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_alltoallw_printinfo: */
int bench_alltoallw_printinfo(bench_t *bench)
{
    printf("Alltoallw\n"
           "  proto: MPI_Alltoallw(sbuf, sendcounts, sdispls, sendtypes, \n"
           "                       rbuf, recvcounts, rdispls, recvtypes, \n"
           "                       MPI_COMM_WORLD)\n"
           "  variable parameter: count\n"
           "  variable parameter default values: min=1, max=%d\n"
           "  For each element: sendtypes[i] = recvtypes[i] = MPI_BYTE\n"
           "  For each element: sendcounts[i] = recvcounts[i] = <count>\n",
           1 << 18);

    return MPIPERF_SUCCESS;
}

/* measure_alltoallw_sync: */
double measure_alltoallw_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Alltoallw(mempool_alloc(sbufpool, sbufsize), sendcounts, sdispls,
                       types, mempool_alloc(rbufpool, rbufsize), recvcounts,
                       rdispls, types, comm);
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        return endtime - starttime;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_TIME_INVVAL;
}

