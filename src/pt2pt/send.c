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
#include "mempool.h"
#include "util.h"

static mempool_t *buf = NULL;
static int bufsize;
static int rank;

/* bench_send_init: */
int bench_send_init(pt2pttest_params_t *params)
{
    bufsize = params->count * sizeof(char);
    buf = mempool_create(params->count, mpiperf_isflushcache);
    if (buf == NULL) {
        return MPIPERF_FAILURE;
    }
	MPI_Comm_rank(params->comm, &rank);
    return MPIPERF_SUCCESS;
}

/* bench_send_free: */
int bench_send_free()
{
    mempool_free(buf);
    return MPIPERF_SUCCESS;
}

/* bench_send_printinfo: */
int bench_send_printinfo()
{
    printf("* Send\n"
           "  proto: MPI_Send(sbuf, count, MPI_BYTE, 1, 0, comm)\n"
           "         MPI_Recv(sbuf, count, MPI_BYTE, 0, 0, comm)\n"
           "  This benchmark measures time of master process only (send side)\n");
    return MPIPERF_SUCCESS;
}

/* measure_send_sync: */
int measure_send_sync(pt2pttest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc = 0;
    
    starttime = timeslot_startsync();
    if (rank == 0) {
        rc = MPI_Send(mempool_alloc(buf, bufsize), params->count, MPI_BYTE, 1, 0,
        		      params->comm);
    } else if (rank == 1) {
        rc = MPI_Recv(mempool_alloc(buf, bufsize), params->count, MPI_BYTE, 0, 0,
        		      params->comm, MPI_STATUS_IGNORE);
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

