/*
 * send.c: Benchmark functions for Send.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "send.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "mpigclock.h"
#include "logger.h"
#include "seq.h"
#include "mempool.h"
#include "stats.h"
#include "util.h"

static mempool_t *buf = NULL;
static int bufsize;
static int count;
static MPI_Comm comm;
static int comm_size;
static int comm_rank;

/* bench_send_init: */
int bench_send_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

    min = (mpiperf_param_min == -1) ? 1 : mpiperf_param_min;
    max = (mpiperf_param_max == -1) ? 65536 : mpiperf_param_max;

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

    buf = mempool_create(max, mpiperf_isflushcache);
    if (buf == NULL) {
        return MPIPERF_FAILURE;
    }

    /*
     * Test communicator: rank 0 (in WORLD) - send; rank 1 - recv,
     * other ranks will not participate in tests.
     */
    MPI_Comm_split(MPI_COMM_WORLD, (mpiperf_rank < 2) ? 1 : MPI_UNDEFINED,
                   mpiperf_rank, &comm);
    if (comm != MPI_COMM_NULL) {
        MPI_Comm_size(comm, &comm_size);
        MPI_Comm_rank(comm, &comm_rank);
    }

    return MPIPERF_SUCCESS;
}

/* bench_send_free: */
int bench_send_free(bench_t *bench)
{
    mempool_free(buf);
    intseq_finalize(bench);
    if (comm != MPI_COMM_NULL) {
        MPI_Comm_free(&comm);
    }
    return MPIPERF_SUCCESS;
}

/* bench_send_init_test: */
int bench_send_init_test(bench_t *bench)
{
    count = bench->paramseq_getcurrent(bench);
    bufsize = count * sizeof(char);
    return MPIPERF_SUCCESS;
}

/* bench_send_free_test: */
int bench_send_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_send_printinfo: */
int bench_send_printinfo(bench_t *bench)
{
    printf("Send\n"
           "  proto: MPI_Send(sbuf, <count>, MPI_BYTE, 1, 0, comm)\n"
           "         MPI_Recv(sbuf, <count>, MPI_BYTE, 0, 0, comm)\n"
           "  variable parameter: count\n"
           "  variable parameter values: min=1, max=65536\n"
           "  This benchmark measures time of master process only (send side)\n");
    return MPIPERF_SUCCESS;
}

MPI_Comm bench_send_getcomm(bench_t *bench)
{
    return comm;
}

int bench_send_getcommsize(bench_t *bench)
{
    return comm_size;
}

int bench_send_getcommrank(bench_t *bench)
{
    return comm_rank;
}

/* measure_send_sync: */
double measure_send_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc = 0;
    
    starttime = timeslot_startsync();
    if (comm_rank == 0) {
        rc = MPI_Send(mempool_alloc(buf, bufsize), count, MPI_BYTE, 1, 0, comm);
    } else if (comm_rank == 1) {
        rc = MPI_Recv(mempool_alloc(buf, bufsize), count, MPI_BYTE, 0, 0, comm,
                      MPI_STATUS_IGNORE);
    }
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

