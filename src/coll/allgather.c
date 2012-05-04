/*
 * allgather.c: Benchmark functions for Allgather.
 *
 * Copyright (C) 2011-2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "allgather.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize;
static int rbufsize;

/* bench_allgather_init: */
int bench_allgather_init(colltest_params_t *params)
{
    sbufpool = mempool_create(params->count, mpiperf_isflushcache);
    rbufpool = mempool_create(params->count * params->nprocs, mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    sbufsize = params->count * sizeof(char);
    rbufsize = params->count * sizeof(char) * params->nprocs;
    return MPIPERF_SUCCESS;
}

/* bench_allgather_free: */
int bench_allgather_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

/* bench_allgather_printinfo: */
int bench_allgather_printinfo()
{
    printf("* Allgather\n"
           "  proto: MPI_Allgather(sbuf, count, MPI_BYTE, \n"
           "                       rbuf, count, MPI_BYTE, comm)\n");
    return MPIPERF_SUCCESS;
}

/* measure_allgather_sync: */
int measure_allgather_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Allgather(mempool_alloc(sbufpool, sbufsize), params->count,
                       MPI_BYTE, mempool_alloc(rbufpool, rbufsize),
                       params->count, MPI_BYTE, params->comm);
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

