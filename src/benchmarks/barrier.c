/*
 * barrier.c: Benchmark functions for Barrier.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "barrier.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"

static MPI_Comm comm;

/* bench_barrier_init: */
int bench_barrier_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

    min = (mpiperf_param_min == -1) ? 1 : mpiperf_param_min;
    max = (mpiperf_param_max == -1) ? mpiperf_commsize : mpiperf_param_max;
    if (min > mpiperf_commsize || min < 1 || max > mpiperf_commsize) {
        print_error("Barrier: incorrect communicator size");
        return MPIPERF_FAILURE;
    }

    if (mpiperf_param_step_type == PARAM_STEP_INC) {
        steptype = SEQ_STEP_INC;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 0) ?
               1 : mpiperf_param_step;
    } else if (mpiperf_param_step_type == PARAM_STEP_MUL) {
        steptype = SEQ_STEP_MUL;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 1) ?
               2 : mpiperf_param_step;
    }
    return commseq_initialize(bench, min, max, step, steptype);
}

/* bench_barrier_free: */
int bench_barrier_free(bench_t *bench)
{
    commseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_barrier_init_test: */
int bench_barrier_init_test(bench_t *bench)
{
    comm = bench->getcomm(bench);
    return MPIPERF_SUCCESS;
}

/* bench_barrier_free_test: */
int bench_barrier_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_barrier_printinfo: */
int bench_barrier_printinfo(bench_t *bench)
{
    printf("Barrier\n"
           "  proto: MPI_Barrier(<comm>)\n"
           "  variable parameter: comm - communicator size\n"
           "  variable parameter default values: min=1, max=MPI_COMM_WORLD\n");
    return MPIPERF_SUCCESS;
}

/* measure_barrier_sync: */
double measure_barrier_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;

    starttime = timeslot_startsync();
    rc = MPI_Barrier(comm);
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

