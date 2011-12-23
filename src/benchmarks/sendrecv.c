/*
 * sendrecv.c: Benchmark functions for Sendrecv.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "sendrecv.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "mpigclock.h"
#include "logger.h"
#include "seq.h"
#include "mempool.h"
#include "stats.h"
#include "util.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static MPI_Comm sendrecv_comm;
static int sendrecv_comm_size;
static int sendrecv_comm_rank;

/* bench_sendrecv_init: */
int bench_sendrecv_init(bench_t *bench)
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

    sbufpool = mempool_create(max, mpiperf_isflushcache);
    rbufpool = mempool_create(max, mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }

    MPI_Comm_split(MPI_COMM_WORLD, (mpiperf_rank < 2) ? 1 : MPI_UNDEFINED,
                   mpiperf_rank, &sendrecv_comm);
    if (sendrecv_comm != MPI_COMM_NULL) {
        MPI_Comm_size(sendrecv_comm, &sendrecv_comm_size);
        MPI_Comm_rank(sendrecv_comm, &sendrecv_comm_rank);
    }

    return MPIPERF_SUCCESS;
}

/* bench_sendrecv_free: */
int bench_sendrecv_free(bench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    intseq_finalize(bench);
    if (sendrecv_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&sendrecv_comm);
    }
    return MPIPERF_SUCCESS;
}

/* bench_sendrecv_init_test: */
int bench_sendrecv_init_test(bench_t *bench)
{

    return MPIPERF_SUCCESS;
}

/* bench_sendrecv_free_test: */
int bench_sendrecv_free_test(bench_t *bench)
{

    return MPIPERF_SUCCESS;
}

/* bench_sendrecv_printinfo: */
int bench_sendrecv_printinfo(bench_t *bench)
{
    printf("Sendrecv\n"
           "  proto: MPI_Sendrecv(sbuf, <count>, MPI_BYTE, 1, 0, rbuf, <count>, MPI_BYTE, 1, 0, comm)\n"
           "  variable parameter: count\n"
           "  variable parameter values: min=1, max=65536\n");
    return MPIPERF_SUCCESS;
}

MPI_Comm bench_sendrecv_getcomm(bench_t *bench)
{
    return sendrecv_comm;
}

int bench_sendrecv_getcommsize(bench_t *bench)
{
    return sendrecv_comm_size;
}

int bench_sendrecv_getcommrank(bench_t *bench)
{
    return sendrecv_comm_rank;
}

/* measure_sendrecv_sync: */
double measure_sendrecv_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc = 0, count, rank;
    
    starttime = timeslot_startsync();
    count  = bench->paramseq_getcurrent(bench);

    if ((rank = bench->getrank(bench)) == 0) {
        rc = MPI_Sendrecv(mempool_alloc(sbufpool, sizeof(char) * count),
                          count, MPI_BYTE, 1, 0,
                          mempool_alloc(rbufpool, sizeof(char) * count),
                          count, MPI_BYTE, 1, 0, bench->getcomm(bench),
                          MPI_STATUS_IGNORE);
    } else if (rank == 1) {
        rc = MPI_Sendrecv(mempool_alloc(sbufpool, sizeof(char) * count),
                          count, MPI_BYTE, 0, 0,
                          mempool_alloc(rbufpool, sizeof(char) * count),
                          count, MPI_BYTE, 0, 0, bench->getcomm(bench),
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

