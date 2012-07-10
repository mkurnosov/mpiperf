/*
 * gatherv.c: Benchmark functions for Gatherv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdlib.h>

#include <mpi.h>

#include "gatherv.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *recvcounts = NULL;
static int *displs = NULL;
static int sbufsize;
static int rbufsize;
static int root = 0;

/* bench_gatherv_init: */
int bench_gatherv_init(colltest_params_t *params)
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

/* bench_gatherv_free: */
int bench_gatherv_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(recvcounts);
    free(displs);
    return MPIPERF_SUCCESS;
}

/* bench_gatherv_printinfo: */
int bench_gatherv_printinfo()
{
    printf("* Gatherv\n"
           "  proto: MPI_Gatherv(sbuf, count, MPI_BYTE, rbuf, recvcounts, displs,\n"
           "                     MPI_BYTE, %d, comm)\n"
           "  For each element: recvcounts[i] = count\n", root);
    return MPIPERF_SUCCESS;
}

/* measure_gatherv_sync: */
int measure_gatherv_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Gatherv(mempool_alloc(sbufpool, sbufsize), params->count, MPI_BYTE,
                     mempool_alloc(rbufpool, rbufsize), recvcounts, displs,
                     MPI_BYTE, root, params->comm);
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

