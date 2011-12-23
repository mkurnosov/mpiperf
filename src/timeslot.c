/*
 * timeslot.c: Timeslots module.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

#include "timeslot.h"
#include "mpigclock.h"
#include "bench.h"
#include "mpiperf.h"
#include "logger.h"
#include "hpctimer.h"
#include "util.h"

#define BCAST_OVERHEAD 1.2

static double timeslot_stagestart;
static int timeslot;
static double timeslot_len;
static double bcasttime;
static double timeslot_slotstart;
static double timeslot_slotstop;

/* timeslot_initialize: */
int timeslot_initialize()
{
    return MPIPERF_SUCCESS;
}

/* timeslot_initialize_test: */
int timeslot_initialize_test(bench_t *bench)
{
    /* Synchronize clocks */
    double synctime = hpctimer_wtime();
    mpigclock_sync(bench->getcomm(bench), mpiperf_master_rank,
                   MPIGCLOCK_SYNC_LINEAR);
    synctime = hpctimer_wtime() - synctime;

    logger_log("Clock synchronization time (commsize: %d, root: %d): %.6f sec.",
               bench->getcommsize(bench), mpiperf_master_rank, synctime);
    logger_log("Local clock offset (commsize: %d, root: %d): %.6f sec.",
               bench->getcommsize(bench), mpiperf_master_rank, mpigclock_offset());

    bcasttime = measure_bcast_double(bench->getcomm(bench)) * BCAST_OVERHEAD;
    logger_log("MPI_Bcast time: %.6f sec.", bcasttime);
    return MPIPERF_SUCCESS;
}

/* timeslot_setlen: Set length of the timeslots in seconds. */
void timeslot_set_length(double length)
{
    timeslot_len = length;
}

/* timeslot_setlen: Set start time of the first time slot. */
double timeslot_set_starttime(bench_t *bench)
{
    MPI_Comm comm = bench->getcomm(bench);
    MPI_Barrier(comm);
    
    timeslot = 0;
    if (IS_MASTER_RANK) {
        timeslot_stagestart = hpctimer_wtime() + bcasttime;
        /*
        logger_log("timeslot: [%.6f ... %.6f] - %.6f sec.",
                   timeslot_starttime, timeslot_starttime + timeslot_len,
                   timeslot_len);
        */
    }
    MPI_Bcast(&timeslot_stagestart, 1, MPI_DOUBLE, mpiperf_master_rank, comm);

    /* Translate global time to local */
    timeslot_stagestart -= mpigclock_offset();
    return timeslot_stagestart;
}

/* timeslot_startsync: Wait for the next timeslot and returns its start time. */
double timeslot_startsync()
{
    double starttime = 0.0;

    if (mpiperf_synctype == MPIPERF_SYNC_TIME) {
    	starttime = timeslot_stagestart + timeslot_len * (timeslot++);
    	if ((timeslot_slotstart = hpctimer_wtime()) > starttime) {
        	return TIMESLOT_TIME_INVALID;
    	} else {
        	while ((timeslot_slotstart = hpctimer_wtime()) < starttime) {
            	/* Wait */ ;
        	}
    	}
    	return timeslot_slotstart;
    }
    return hpctimer_wtime();
}

/*
 * timeslot_stopsync: Returns timestamp or TIMESLOT_TIME_INVALID
 *                    if current timeslot have finished.
 */
double timeslot_stopsync()
{
    if (mpiperf_synctype == MPIPERF_SYNC_TIME) {
        timeslot_slotstop = hpctimer_wtime();
    	return (timeslot_slotstop - timeslot_slotstart < timeslot_len) ?
               timeslot_slotstop : TIMESLOT_TIME_INVALID;
    }
    return hpctimer_wtime();
}

/* timeslot_finalize: */
void timeslot_finalize()
{
}

