/*
 * scan.c: Benchmark functions for Scan.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "scan.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int sbufsize, rbufsize;

/* bench_scan_init: */
int bench_scan_init(colltest_params_t *params)
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

/* bench_scan_free: */
int bench_scan_free(collbench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    return MPIPERF_SUCCESS;
}

/* bench_scan_printinfo: */
int bench_scan_printinfo()
{
    printf("* Scan\n"
           "  proto: MPI_Scan(sbuf, rbuf, count, MPI_DOUBLE, \n"
           "                  MPI_SUM, comm)\n");
    return MPIPERF_SUCCESS;
}

/* measure_scan_sync: */
int measure_scan_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Scan(mempool_alloc(sbufpool, sbufsize),
                  mempool_alloc(rbufpool, rbufsize), params->count, MPI_DOUBLE,
                  MPI_SUM, params->comm);
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

