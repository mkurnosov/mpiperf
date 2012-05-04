/*
 * barrier.c: Benchmark functions for Barrier.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "barrier.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"

/* bench_barrier_init: */
int bench_barrier_init(colltest_params_t *params)
{
    return MPIPERF_SUCCESS;
}

/* bench_barrier_free: */
int bench_barrier_free()
{
    return MPIPERF_SUCCESS;
}

/* bench_barrier_printinfo: */
int bench_barrier_printinfo()
{
    printf("* Barrier\n"
           "  proto: MPI_Barrier(comm)\n");
    return MPIPERF_SUCCESS;
}

/* measure_barrier_sync: */
int measure_barrier_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;

    starttime = timeslot_startsync();
    rc = MPI_Barrier(params->comm);
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

