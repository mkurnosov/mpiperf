/*
 * reduce_scatter.c: Benchmark functions for Reduce_scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "reduce_scatter.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *recvcounts = NULL;
static int count;
static int sbufsize;
static int rbufsize;
static int commsize;
static int commrank;
static MPI_Comm comm;

/* bench_reduce_scatter_init: */
int bench_reduce_scatter_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

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

    recvcounts = malloc(sizeof(*recvcounts) * bench->getcommsize(bench));
    if (recvcounts == NULL) {
        return MPIPERF_FAILURE;
    }

    sbufpool = mempool_create(max * sizeof(double), mpiperf_isflushcache);
    rbufpool = mempool_create(max * sizeof(double) * bench->getcommsize(bench),
                              mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    commsize = bench->getcommsize(bench);
    comm = bench->getcomm(bench);
    commrank = bench->getrank(bench);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_free: */
int bench_reduce_scatter_free(bench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(recvcounts);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_init_test: */
int bench_reduce_scatter_init_test(bench_t *bench)
{
    int i;

    count = bench->paramseq_getcurrent(bench);
    for (i = 0; i < commsize; i++) {
        recvcounts[i] = count / commsize;
    }
    for (i = 0; i < count % commsize; i++) {
        recvcounts[i * commsize / (count % commsize)]++;
    }

    sbufsize = count * sizeof(double);
    rbufsize = recvcounts[commrank] * sizeof(double);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_free_test: */
int bench_reduce_scatter_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_printinfo: */
int bench_reduce_scatter_printinfo(bench_t *bench)
{
    printf("Reduce_scatter\n"
           "  proto: MPI_Reduce_scatter(sbuf, rbuf, recvcounts, MPI_DOUBLE, \n"
           "                            MPI_SUM, MPI_COMM_WORLD)\n"
           "  variable parameter: count = sum{recvcounts[i]}\n"
           "  variable parameter default values: min=1, max=%d\n"
           "  Send buffer is divided onto equal parts and scattered among processes\n",
           1 << 18);
    return MPIPERF_SUCCESS;
}

/* measure_reduce_scatter_sync: */
double measure_reduce_scatter_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Reduce_scatter(mempool_alloc(sbufpool, sbufsize),
                            mempool_alloc(rbufpool, rbufsize), recvcounts,
                            MPI_DOUBLE, MPI_SUM, comm);
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

