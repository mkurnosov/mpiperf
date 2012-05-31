/*
 * igather.c: Benchmark functions for Igather.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "igather.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;
static int root = 0;

int bench_igather_init(nbctest_params_t *params)
{
    int rank;

    rbufpool = NULL;
    rbufsize = 0;

    sbufpool = mempool_create(params->count, mpiperf_isflushcache);
    if (sbufpool == NULL) {
        return MPIPERF_FAILURE;
    }
    sbufsize = params->count * sizeof(char);

    MPI_Comm_rank(params->comm, &rank);
    if (rank == root) {
        rbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
        if (rbufpool == NULL) {
            mempool_free(sbufpool);
            return MPIPERF_FAILURE;
        }
        rbufsize = params->count * sizeof(char);
    }
    return MPIPERF_SUCCESS;
}

int bench_igather_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

int bench_igather_printinfo()
{
    printf("* Igather\n"
           "  proto: MPI_Igather(sbuf, count, MPI_BYTE, \n"
           "                     rbuf, count, MPI_BYTE, %d, comm)\n", root);
    return MPIPERF_SUCCESS;
}

int measure_igather_blocking(nbctest_params_t *params,
                             nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Igather(mempool_alloc(sbufpool, sbufsize), params->count, MPI_BYTE,
                      mempool_alloc(rbufpool, rbufsize), params->count, MPI_BYTE,
                      root, params->comm, &req);
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

int measure_igather_overlap(nbctest_params_t *params,
                            nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Igather(mempool_alloc(sbufpool, sbufsize), params->count, MPI_BYTE,
                      mempool_alloc(rbufpool, rbufsize), params->count, MPI_BYTE,
                      root, params->comm, &req);
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
#endif
    return MEASURE_FAILURE;
}
