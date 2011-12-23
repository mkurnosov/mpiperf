/*
 * allreduce.c: Benchmark functions for Allreduce.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "allreduce.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int count, sbufsize, rbufsize;
static int commsize;
static MPI_Comm comm;

/* bench_allreduce_init: */
int bench_allreduce_init(bench_t *bench)
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

    sbufpool = mempool_create(max * sizeof(double), mpiperf_isflushcache);
    rbufpool = mempool_create(max * sizeof(double), mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    return MPIPERF_SUCCESS;
}

/* bench_allreduce_free: */
int bench_allreduce_free(bench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_allreduce_init_test: */
int bench_allreduce_init_test(bench_t *bench)
{
    count = bench->paramseq_getcurrent(bench);
    sbufsize = count * sizeof(double);
    rbufsize = count * sizeof(double);
    return MPIPERF_SUCCESS;
}

/* bench_allreduce_free_test: */
int bench_allreduce_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_allreduce_printinfo: */
int bench_allreduce_printinfo(bench_t *bench)
{
    printf("Allreduce\n"
           "  proto: MPI_Allreduce(sbuf, rbuf, <count>, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD)\n"
           "  variable parameter: count\n"
           "  variable parameter default values: min=1, max=%d\n", 1 << 18);
    return MPIPERF_SUCCESS;
}

/* measure_allreduce_sync: */
double measure_allreduce_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Allreduce(mempool_alloc(sbufpool, sbufsize),
                       mempool_alloc(rbufpool, rbufsize), count, MPI_DOUBLE,
                       MPI_SUM, comm);
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

