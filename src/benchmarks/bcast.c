/*
 * bcast.c: Benchmark functions for Bcast.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "bcast.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *bufpool = NULL;
static int count;
static int bufsize;
static int commsize;
static MPI_Comm comm;
static int root = 0;

/* bench_bcast_init: */
int bench_bcast_init(bench_t *bench)
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

    bufpool = mempool_create(max, mpiperf_isflushcache);
    if (bufpool == NULL) {
        return MPIPERF_FAILURE;
    }
    return MPIPERF_SUCCESS;
}

/* bench_bcast_free: */
int bench_bcast_free(bench_t *bench)
{
    mempool_free(bufpool);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_bcast_init_test: */
int bench_bcast_init_test(bench_t *bench)
{
    count = bench->paramseq_getcurrent(bench);
    bufsize = count * sizeof(char);
    return MPIPERF_SUCCESS;
}

/* bench_bcast_free_test: */
int bench_bcast_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_bcast_printinfo: */
int bench_bcast_printinfo(bench_t *bench)
{
    printf("Bcast\n"
           "  proto: MPI_Bcast(buf, <count>, MPI_BYTE, %d, MPI_COMM_WORLD)\n"
           "  variable parameter: count\n"
           "  variable parameter default values: min=1, max=%d\n", root, 1 << 18);
    return MPIPERF_SUCCESS;
}

/* measure_bcast_sync: */
double measure_bcast_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Bcast(mempool_alloc(bufpool, bufsize), count, MPI_BYTE, root, comm);
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

