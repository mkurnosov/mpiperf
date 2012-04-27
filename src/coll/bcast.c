/*
 * bcast.c: Benchmark functions for Bcast.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#include "bcast.h"
#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "util.h"
#include "mempool.h"

static mempool_t *bufpool = NULL;
static int bufsize;
static int root = 0;

/* bench_bcast_init: */
int bench_bcast_init(colltest_params_t *params)
{
    bufpool = mempool_create(params->count, mpiperf_isflushcache);
    if (bufpool == NULL) {
        return MPIPERF_FAILURE;
    }
    bufsize = params->count * sizeof(char);
    return MPIPERF_SUCCESS;
}

/* bench_bcast_free: */
int bench_bcast_free()
{
    mempool_free(bufpool);
    return MPIPERF_SUCCESS;
}

/* bench_bcast_printinfo: */
int bench_bcast_printinfo()
{
    printf("* Bcast\n"
           "  proto: MPI_Bcast(buf, count, MPI_BYTE, %d, comm)\n", root);
    return MPIPERF_SUCCESS;
}

/* measure_bcast_sync: */
int measure_bcast_sync(colltest_params_t *params, double *time)
{
    double starttime, endtime;
    int rc;
    
    starttime = timeslot_startsync();
    rc = MPI_Bcast(mempool_alloc(bufpool, bufsize), params->count, MPI_BYTE,
    		       root, params->comm);
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

