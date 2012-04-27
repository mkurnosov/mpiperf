/*
 * waitpattern.c: Benchmark functions for wait routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <mpi.h>

#include "waitpattern.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"

static int rank;

/* bench_waitpatternup_init: */
int bench_waitpattern_init(colltest_params_t *params)
{
    MPI_Comm_rank(params->comm, &rank);
	return MPIPERF_SUCCESS;
}

/* bench_waitpatternup_free: */
int bench_waitpattern_free()
{
    return MPIPERF_SUCCESS;
}

/* bench_waitpatternup_printinfo: */
int bench_waitpatternup_printinfo()
{
    printf("* WaitPatternUp\n"
           "  proto: WaitPatternUp(comm)\n"
           "  Description: process i waits (i + 1) usec\n"
           "  Expected benchmark time is <comm> usec. \n");
    return MPIPERF_SUCCESS;
}

/* measure_waitpatternup_sync: */
int measure_waitpatternup_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime, deadline;

    starttime = timeslot_startsync();
    for (deadline = hpctimer_wtime() + (rank + 1) * 1e-6;
         hpctimer_wtime() < deadline; )
    {
        /* Wait */
    }
    endtime = timeslot_stopsync();

    if ((starttime > 0.0) && (endtime > 0.0)) {
        *time = endtime - starttime;
    	return MEASURE_SUCCESS;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_FAILURE;
}

/* bench_waitpatterndown_printinfo: */
int bench_waitpatterndown_printinfo()
{
    printf("* WaitPatternDown\n"
           "  proto: WaitPatternDown(comm)\n"
           "  Description: process i waits (<comm> - i) usec\n"
           "  Expected benchmark time is <comm> usec. \n");
    return MPIPERF_SUCCESS;
}

/* measure_waitpatterndown_sync: */
int measure_waitpatterndown_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime, deadline;

    starttime = timeslot_startsync();
    for (deadline = hpctimer_wtime() + (params->nprocs - rank) * 1e-6;
         hpctimer_wtime() < deadline; )
    {
        /* Wait */
    }
    endtime = timeslot_stopsync();

    if ((starttime > 0.0) && (endtime > 0.0)) {
        *time = endtime - starttime;
    	return MEASURE_SUCCESS;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_FAILURE;
}

/* bench_waitpatternnull_printinfo: */
int bench_waitpatternnull_printinfo()
{
    printf("* WaitPatternNull\n"
           "  proto: WaitPatternNull(comm)\n"
           "  Description: process i waits 0 usec\n"
           "  Expected benchmark time is 0 usec. \n");
    return MPIPERF_SUCCESS;
}

/* measure_waitpatternnull_sync: */
int measure_waitpatternnull_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;

    starttime = timeslot_startsync();
    /* void */
    endtime = timeslot_stopsync();

    if ((starttime > 0.0) && (endtime > 0.0)) {
        *time = endtime - starttime;
    	return MEASURE_SUCCESS;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_FAILURE;
}
