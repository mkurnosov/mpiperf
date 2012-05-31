/*
 * igatherv.c: Benchmark functions for Igatherv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "igatherv.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *recvcounts = NULL;
static int *displs = NULL;
static int sbufsize;
static int rbufsize;
static int root = 0;

int bench_igatherv_init(nbctest_params_t *params)
{
    int rank, i;

    rbufpool = NULL;
    rbufsize = 0;
    recvcounts = NULL;
    displs = NULL;

    sbufpool = mempool_create(params->count, mpiperf_isflushcache);
    if (sbufpool == NULL) {
        goto errhandler;
    }
    sbufsize = params->count * sizeof(char);

    MPI_Comm_rank(params->comm, &rank);
    if (rank == root) {
        recvcounts = malloc(sizeof(*recvcounts) * params->nprocs);
        displs = malloc(sizeof(*displs) * params->nprocs);
        if (recvcounts == NULL || displs == NULL) {
            goto errhandler;
        }

        rbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
        if (rbufpool == NULL) {
            goto errhandler;
        }
        rbufsize = params->count * params->nprocs * sizeof(char);
        for (i = 0; i < params->nprocs; i++) {
            recvcounts[i] = params->count;
        }
        displs[0] = 0;
        for (i = 1; i < params->nprocs; i++) {
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }
    return MPIPERF_SUCCESS;

errhandler:
    free(recvcounts);
    free(displs);
    mempool_free(sbufpool);
    mempool_free(rbufpool);

    return MPIPERF_FAILURE;
}

int bench_igatherv_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(recvcounts);
    free(displs);
    return MPIPERF_SUCCESS;
}

int bench_igatherv_printinfo()
{
    printf("* Igatherv\n"
           "  proto: MPI_Igatherv(sbuf, count, MPI_BYTE, rbuf, recvcounts, displs,\n"
           "                      MPI_BYTE, %d, comm)\n"
           "  For each element: recvcounts[i] = count\n", root);
    return MPIPERF_SUCCESS;
}

int measure_igatherv_blocking(nbctest_params_t *params,
                              nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Igatherv(mempool_alloc(sbufpool, sbufsize), params->count, MPI_BYTE,
                       mempool_alloc(rbufpool, rbufsize), recvcounts, displs,
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

int measure_igatherv_overlap(nbctest_params_t *params,
                             nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Igatherv(mempool_alloc(sbufpool, sbufsize), params->count, MPI_BYTE,
                       mempool_alloc(rbufpool, rbufsize), recvcounts, displs,
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
#endif
    return MEASURE_FAILURE;
}
