/*
 * waitpattern.c: Benchmark functions for wait routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "waitpattern.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "mpigclock.h"
#include "hpctimer.h"
#include "logger.h"
#include "seq.h"
#include "mempool.h"
#include "stats.h"
#include "util.h"

static int rank;
static int commsize;

/* bench_waitpatternup_init: */
int bench_waitpattern_init(bench_t *bench)
{
    int min, max, step = 0, steptype = -1;

    min = (mpiperf_param_min == -1) ? 1 : mpiperf_param_min;
    max = (mpiperf_param_max == -1) ? mpiperf_commsize : mpiperf_param_max;
    if (min > mpiperf_commsize || min < 1) {
        print_error("Incorrect communicator size");
        return MPIPERF_FAILURE;
    }
    if (max > mpiperf_commsize) {
        print_error("Communicator size is too small");
        return MPIPERF_FAILURE;
    }

    if (mpiperf_param_step_type == PARAM_STEP_INC) {
        steptype = SEQ_STEP_INC;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 0) ?
               1 : mpiperf_param_step;
    } else if (mpiperf_param_step_type == PARAM_STEP_MUL) {
        steptype = SEQ_STEP_MUL;
        step = (mpiperf_param_step == -1 || mpiperf_param_step == 1) ?
               2 : mpiperf_param_step;
    }
    return commseq_initialize(bench, min, max, step, steptype);
}

/* bench_waitpatternup_free: */
int bench_waitpattern_free(bench_t *bench)
{
    commseq_finalize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_waitpatternup_init_test: */
int bench_waitpattern_init_test(bench_t *bench)
{
    rank = bench->getrank(bench);
    commsize = bench->getcommsize(bench);
    return MPIPERF_SUCCESS;
}

/* bench_waitpatternup_free_test: */
int bench_waitpattern_free_test(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* bench_waitpatternup_printinfo: */
int bench_waitpatternup_printinfo(bench_t *bench)
{
    printf("WaitPatternUp\n"
           "  proto: WaitPatternUp(<comm>)\n"
           "  variable parameter: comm - communicator size\n"
           "  Description: process i waits (i + 1) usec\n");
    return MPIPERF_SUCCESS;
}

/* measure_waitpatternup_sync: */
double measure_waitpatternup_sync(bench_t *bench)
{
    double starttime, endtime, deadline;

    starttime = timeslot_startsync();
    for (deadline = hpctimer_wtime() + (rank + 1) * 1e-6;
         hpctimer_wtime() < deadline; )
    {
        /* Wait */
    }
    endtime = timeslot_stopsync();

    if (starttime > 0.0 && endtime > 0.0) {
        return endtime - starttime;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_TIME_INVVAL;
}

/* bench_waitpatterndown_printinfo: */
int bench_waitpatterndown_printinfo(bench_t *bench)
{
    printf("WaitPatternDown\n"
           "  proto: WaitPatternDown(<comm>)\n"
           "  variable parameter: comm - communicator size\n"
           "  Description: process i waits (<comm> - i) usec\n");
    return MPIPERF_SUCCESS;
}

/* measure_waitpatterndown_sync: */
double measure_waitpatterndown_sync(bench_t *bench)
{
    double starttime, endtime, deadline;

    starttime = timeslot_startsync();
    for (deadline = hpctimer_wtime() + (commsize - rank) * 1e-6;
         hpctimer_wtime() < deadline; )
    {
        /* Wait for 1 us */
    }
    endtime = timeslot_stopsync();

    if (starttime > 0.0 && endtime > 0.0) {
        return endtime - starttime;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_TIME_INVVAL;
}

/* bench_waitpatternnull_printinfo: */
int bench_waitpatternnull_printinfo(bench_t *bench)
{
    printf("WaitPatternNull\n"
           "  proto: WaitPatternNull(<comm>)\n"
           "  variable parameter: comm - communicator size\n"
           "  Description: process i waits 0 usec\n");
    return MPIPERF_SUCCESS;
}

/* measure_waitpatternnull_sync: */
double measure_waitpatternnull_sync(bench_t *bench)
{
    double starttime, endtime;

    starttime = timeslot_startsync();
    /* Empty */
    endtime = timeslot_stopsync();

    if (starttime > 0.0 && endtime > 0.0) {
        return endtime - starttime;
    } else if (starttime < 0.0) {
        return MEASURE_STARTED_LATE;
    } else if (endtime < 0.0) {
        return MEASURE_TIME_TOOLONG;
    }
    return MEASURE_TIME_INVVAL;
}
