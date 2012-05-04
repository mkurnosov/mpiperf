/*
 * reduce_scatter.c: Benchmark functions for Reduce_scatter.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "reduce_scatter.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *recvcounts = NULL;
static int sbufsize;
static int rbufsize;

/* bench_reduce_scatter_init: */
int bench_reduce_scatter_init(colltest_params_t *params)
{
	int i, rank;

	recvcounts = malloc(sizeof(*recvcounts) * params->nprocs);
    if (recvcounts == NULL) {
        return MPIPERF_FAILURE;
    }

    sbufpool = mempool_create(params->count * sizeof(double), mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * sizeof(double) * params->nprocs,
                              mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }

    for (i = 0; i < params->nprocs; i++) {
        recvcounts[i] = params->count / params->nprocs;
    }
    for (i = 0; i < params->count % params->nprocs; i++) {
        recvcounts[i * params->nprocs / (params->count % params->nprocs)]++;
    }
    MPI_Comm_rank(params->comm, &rank);
    sbufsize = params->count * sizeof(double);
    rbufsize = recvcounts[rank] * sizeof(double);

    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_free: */
int bench_reduce_scatter_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(recvcounts);
    return MPIPERF_SUCCESS;
}

/* bench_reduce_scatter_printinfo: */
int bench_reduce_scatter_printinfo()
{
    printf("* Reduce_scatter\n"
           "  proto: MPI_Reduce_scatter(sbuf, rbuf, recvcounts, MPI_DOUBLE, \n"
           "                            MPI_SUM, comm)\n"
           "  Send buffer is divided onto equal parts and scattered among processes\n");
    return MPIPERF_SUCCESS;
}

/* measure_reduce_scatter_sync: */
int measure_reduce_scatter_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Reduce_scatter(mempool_alloc(sbufpool, sbufsize),
                            mempool_alloc(rbufpool, rbufsize), recvcounts,
                            MPI_DOUBLE, MPI_SUM, params->comm);
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

