/*
 * scatterv.c: Benchmark functions for Scatterv.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdlib.h>

#include <mpi.h>

#include "scatterv.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *sendcounts = NULL;
static int *displs = NULL;
static int sbufsize;
static int rbufsize;
static int root = 0;

/* bench_scatterv_init: */
int bench_scatterv_init(colltest_params_t *params)
{
    int rank, i;

	sbufpool = NULL;
	sbufsize = 0;
    sendcounts = NULL;
	displs = NULL;

    MPI_Comm_rank(params->comm, &rank);
    if (rank == root) {
    	sendcounts = malloc(sizeof(*sendcounts) * params->nprocs);
        displs = malloc(sizeof(*displs) * params->nprocs);
        if (sendcounts == NULL || displs == NULL) {
            goto errhandler;
        }

        sbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
        if (sbufpool == NULL) {
            goto errhandler;
        }
        sbufsize = params->count * params->nprocs * sizeof(char);

        for (i = 0; i < params->nprocs; i++) {
            sendcounts[i] = params->count;
        }
        displs[0] = 0;
        for (i = 1; i < params->nprocs; i++) {
            displs[i] = displs[i - 1] + sendcounts[i - 1];
        }
    }

    rbufpool = mempool_create(params->count, mpiperf_isflushcache);
    if (rbufpool == NULL) {
        goto errhandler;
    }
    rbufsize = params->count * sizeof(char);

    return MPIPERF_SUCCESS;

errhandler:
    free(sendcounts);
    free(displs);
    mempool_free(sbufpool);
    mempool_free(rbufpool);

    return MPIPERF_FAILURE;
}

/* bench_scatterv_free: */
int bench_scatterv_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(sendcounts);
    free(displs);
    return MPIPERF_SUCCESS;
}

/* bench_scatterv_printinfo: */
int bench_scatterv_printinfo()
{
    printf("* Scatterv\n"
           "  proto: MPI_Scatterv(sbuf, sendcounts, displs, MPI_BYTE, \n"
           "                      rbuf, count, MPI_BYTE, %d, comm)\n"
           "  For each element: sendcounts[i] = count\n", root);
    return MPIPERF_SUCCESS;
}

/* measure_scatterv_sync: */
int measure_scatterv_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Scatterv(mempool_alloc(sbufpool, sbufsize), sendcounts, displs,
                      MPI_BYTE, mempool_alloc(rbufpool, rbufsize), params->count,
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

