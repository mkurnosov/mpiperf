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
#include "mempool.h"
#include "util.h"

static mempool_t *sbuf = NULL;
static mempool_t *rbuf = NULL;
static int sbufsize;
static int rbufsize;
static int rank;

/* bench_sendrecv_init: */
int bench_sendrecv_init(pt2pttest_params_t *params)
{
    sbufsize = params->count * sizeof(char);
    rbufsize = params->count * sizeof(char);
    sbuf = mempool_create(params->count, mpiperf_isflushcache);
    rbuf = mempool_create(params->count, mpiperf_isflushcache);
    if (sbuf == NULL || rbuf == NULL) {
        mempool_free(sbuf);
        mempool_free(rbuf);
        return MPIPERF_FAILURE;
    }
    MPI_Comm_rank(params->comm, &rank);
    return MPIPERF_SUCCESS;
}

/* bench_sendrecv_free: */
int bench_sendrecv_free()
{
    mempool_free(sbuf);
    mempool_free(rbuf);
    return MPIPERF_SUCCESS;
}

/* bench_sendrecv_printinfo: */
int bench_sendrecv_printinfo()
{
    printf("* Sendrecv\n"
           "  proto: MPI_Sendrecv(sbuf, count, MPI_BYTE, 1, 0, rbuf, count, MPI_BYTE, 1, 0, comm)\n"
           "  This benchmark measures time of master process only\n");
    return MPIPERF_SUCCESS;
}

/* measure_sendrecv_sync: */
int measure_sendrecv_sync(pt2pttest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc = 0;
    
    starttime = timeslot_startsync();
    if (rank == 0) {
        rc = MPI_Sendrecv(mempool_alloc(sbuf, sizeof(char) * params->count),
                          params->count, MPI_BYTE, 1, 0,
                          mempool_alloc(rbuf, sizeof(char) * params->count),
                          params->count, MPI_BYTE, 1, 0, params->comm,
                          MPI_STATUS_IGNORE);
    } else if (rank == 1) {
        rc = MPI_Sendrecv(mempool_alloc(sbuf, sizeof(char) * params->count),
                          params->count, MPI_BYTE, 0, 0,
                          mempool_alloc(rbuf, sizeof(char) * params->count),
                          params->count, MPI_BYTE, 0, 0, params->comm,
                          MPI_STATUS_IGNORE);
    }
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

