/*
 * scatterv.c: Benchmark functions for Scatterv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "scatterv.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *sendcounts = NULL;
static int *displs = NULL;
static int count;
static int sbufsize;
static int rbufsize;
static int commsize;
static int commrank;
static MPI_Comm comm;
static int root = 0;

/* bench_scatterv_init: */
int bench_scatterv_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

    commsize = bench->getcommsize(bench);
    comm = bench->getcomm(bench);
    commrank = bench->getrank(bench);

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

    sendcounts = malloc(sizeof(*sendcounts) * bench->getcommsize(bench));
    displs = malloc(sizeof(*displs) * bench->getcommsize(bench));
    if (sendcounts == NULL || displs == NULL) {
        goto errhandler;
    }

    rbufpool = mempool_create(max, mpiperf_isflushcache);
    if (rbufpool == NULL) {
        goto errhandler;
    }
    if (commrank == root) {
        sbufpool = mempool_create(max * bench->getcommsize(bench),
                                  mpiperf_isflushcache);
        if (sbufpool == NULL) {
            goto errhandler;
        }
    }
    return MPIPERF_SUCCESS;

errhandler:
    free(sendcounts);
    free(displs);
    mempool_free(sbufpool);
    mempool_free(rbufpool);

    return MPIPERF_FAILURE;
}

/* bench_scatterv_free: */
int bench_scatterv_free(bench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(sendcounts);
    free(displs);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_scatterv_init_test: */
int bench_scatterv_init_test(bench_t *bench)
{
    int i;

    count = bench->paramseq_getcurrent(bench);
    for (i = 0; i < commsize; i++) {
        sendcounts[i] = count;
    }

    displs[0] = 0;
    for (i = 1; i < commsize; i++) {
        displs[i] = displs[i - 1] + sendcounts[i - 1];
    }

    sbufsize = (commrank == root) ? count * commsize : 0;
    rbufsize = count;
    return MPIPERF_SUCCESS;
}

/* bench_scatterv_free_test: */
int bench_scatterv_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_scatterv_printinfo: */
int bench_scatterv_printinfo(bench_t *bench)
{
    printf("Scatterv\n"
           "  proto: MPI_Scatterv(sbuf, sendcounts, displs, MPI_BYTE, \n"
           "                      rbuf, <count>, MPI_BYTE, %d, MPI_COMM_WORLD)\n"
           "  variable parameter: count\n"
           "  variable parameter default values: min=1, max=%d\n"
           "  For each element: sendcounts[i] = <count>\n",
           root, 1 << 18);
    return MPIPERF_SUCCESS;
}

/* measure_scatterv_sync: */
double measure_scatterv_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Scatterv(mempool_alloc(sbufpool, sbufsize), sendcounts, displs,
                      MPI_BYTE, mempool_alloc(rbufpool, rbufsize), count,
                      MPI_BYTE, root, comm);
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

