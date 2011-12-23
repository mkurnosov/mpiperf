/*
 * scan.c: Benchmark functions for Scan.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "scan.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "seq.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int count, sbufsize, rbufsize;
static int commsize;
static MPI_Comm comm;

/* bench_scan_init: */
int bench_scan_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

    min = (mpiperf_param_min == -1) ? 1 : mpiperf_param_min;
    max = (mpiperf_param_max == -1) ? 1 << 18 : mpiperf_param_max;

    if (mpiperf_param_step_type == PARAM_STEP_INC) {
        steptype = SEQ_STEP_INC;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 0) ?
               64 : mpiperf_param_step;
    } else if (mpiperf_param_step_type == PARAM_STEP_MUL) {
        steptype = SEQ_STEP_MUL;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 1) ?
               2 : mpiperf_param_step;
    }
    if (intseq_initialize(bench, min, max, step, steptype) == MPIPERF_FAILURE)
        return MPIPERF_FAILURE;

    sbufpool = mempool_create(max * sizeof(double), mpiperf_isflushcache);
    rbufpool = mempool_create(max * sizeof(double), mpiperf_isflushcache);
    if (sbufpool == NULL || rbufpool == NULL) {
        mempool_free(sbufpool);
        mempool_free(rbufpool);
        return MPIPERF_FAILURE;
    }
    commsize = bench->getcommsize(bench);
    comm = bench->getcomm(bench);
    return MPIPERF_SUCCESS;
}

/* bench_scan_free: */
int bench_scan_free(bench_t *bench)
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    intseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_scan_init_test: */
int bench_scan_init_test(bench_t *bench)
{
    count = bench->paramseq_getcurrent(bench);
    sbufsize = count * sizeof(double);
    rbufsize = count * sizeof(double);
    return MPIPERF_SUCCESS;
}

/* bench_scan_free_test: */
int bench_scan_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_scan_printinfo: */
int bench_scan_printinfo(bench_t *bench)
{
    printf("Scan\n"
           "  proto: MPI_Scan(sbuf, rbuf, <count>, MPI_DOUBLE, \n"
           "                  MPI_SUM, MPI_COMM_WORLD)\n"
           "  variable parameter: count\n"
           "  variable parameter default values: min=1, max=%d\n", 1 << 18);
    return MPIPERF_SUCCESS;
}

/* measure_scan_sync: */
double measure_scan_sync(bench_t *bench)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Scan(mempool_alloc(sbufpool, sbufsize),
                  mempool_alloc(rbufpool, rbufsize), count, MPI_DOUBLE,
                  MPI_SUM, comm);
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
        return endtime - starttime;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_TIME_INVVAL;
}

