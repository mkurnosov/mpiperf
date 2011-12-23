/*
 * gather.c: Benchmark functions for Gather.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "gather.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sendcount, sbufsize, rbufsize;
static int commsize;
static int commrank;
static MPI_Comm comm;
static int root = 0;

/* bench_gather_init: */
int bench_gather_init(bench_t *bench)
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

    sbufpool = mempool_create(max, mpiperf_isflushcache);
    if (sbufpool == NULL) {
        return MPIPERF_FAILURE;
    }
    if (commrank == root) {
        rbufpool = mempool_create(max * bench->getcommsize(bench),
                                  mpiperf_isflushcache);
        if (rbufpool == NULL) {
            mempool_free(sbufpool);
            return MPIPERF_FAILURE;
        }
    }
    return MPIPERF_SUCCESS;
}

/* bench_gather_free: */
int bench_gather_free(bench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_gather_init_test: */
int bench_gather_init_test(bench_t *bench)
{
    sendcount = bench->paramseq_getcurrent(bench);
    sbufsize = sendcount * sizeof(char);
    rbufsize = (commrank == root) ? sendcount * sizeof(char) * commsize : 0;
    return MPIPERF_SUCCESS;
}

/* bench_gather_free_test: */
int bench_gather_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_gather_printinfo: */
int bench_gather_printinfo(bench_t *bench)
{
    printf("Gather\n"
           "  proto: MPI_Gather(sbuf, <count>, MPI_BYTE, \n"
           "                    rbuf, <count>, MPI_BYTE, %d, MPI_COMM_WORLD)\n"
           "  variable parameter: count\n"
           "  variable parameter default values: min=1, max=%d\n", root, 1 << 18);
    return MPIPERF_SUCCESS;
}

/* measure_gather_sync: */
double measure_gather_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Gather(mempool_alloc(sbufpool, sbufsize), sendcount, MPI_BYTE,
                    mempool_alloc(rbufpool, rbufsize), sendcount, MPI_BYTE,
                    root, comm);
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

