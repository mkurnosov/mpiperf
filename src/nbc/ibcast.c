/*
 * ibcast.c: Benchmark functions for Ibcast.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "ibcast.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *bufpool = NULL;
static int bufsize;
static int root = 0;

int bench_ibcast_init(nbctest_params_t *params)
{
    bufpool = mempool_create(params->count, mpiperf_isflushcache);
    if (bufpool == NULL)
        return MPIPERF_FAILURE;
    bufsize = params->count * sizeof(char);
    return MPIPERF_SUCCESS;
}

int bench_ibcast_free()
{
    mempool_free(bufpool);
    return MPIPERF_SUCCESS;
}

int bench_ibcast_printinfo()
{
    printf("* Ibcast\n"
           "  proto: MPI_Ibcast(buf, count, MPI_BYTE, %d, comm)\n", root);
    return MPIPERF_SUCCESS;
}

int measure_ibcast_blocking(nbctest_params_t *params,
		                        nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Ibcast(mempool_alloc(bufpool, bufsize), params->count,
                     MPI_BYTE, root, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
    	result->totaltime = endtime - starttime;
		return MEASURE_SUCCESS;
    }
#endif
    return MEASURE_FAILURE;
}

int measure_ibcast_overlap(nbctest_params_t *params,
		                       nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Ibcast(mempool_alloc(bufpool, bufsize), params->count,
                     MPI_BYTE, root, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    nbcbench_simulate_computing(params, &req, result);
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
    	result->totaltime = endtime - starttime;
		return MEASURE_SUCCESS;
    }
    return MEASURE_FAILURE;
#endif
}

