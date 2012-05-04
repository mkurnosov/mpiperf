/*
 * iexscan.c: Benchmark functions for Iexscan.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "iexscan.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;

int bench_iexscan_init(nbctest_params_t *params)
{
    sbufpool = mempool_create(params->count * sizeof(double), mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * sizeof(double), mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    sbufsize = params->count * sizeof(double);
    rbufsize = params->count * sizeof(double);
    return MPIPERF_SUCCESS;
}

int bench_iexscan_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

int bench_iexscan_printinfo()
{
    printf("* Iexscan\n"
           "  proto: MPI_Iexscan(sbuf, rbuf, count, MPI_DOUBLE, \n"
           "                     MPI_SUM, comm)\n");
    return MPIPERF_SUCCESS;
}

int measure_iexscan_blocking(nbctest_params_t *params,
		                     nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Iexscan(mempool_alloc(sbufpool, sbufsize),
                      mempool_alloc(rbufpool, sbufsize), params->count,
                      MPI_DOUBLE, MPI_SUM, params->comm, &req);
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

int measure_iexscan_overlap(nbctest_params_t *params,
		                    nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Iexscan(mempool_alloc(sbufpool, sbufsize),
                      mempool_alloc(rbufpool, sbufsize), params->count,
                      MPI_DOUBLE, MPI_SUM, params->comm, &req);
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

