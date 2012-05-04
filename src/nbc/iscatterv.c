/*
 * iscatterv.c: Benchmark functions for Iscatterv.
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include "iscatterv.h"
#include "bench_nbc.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "hpctimer.h"
#include "util.h"
#include "mempool.h"

static mempool_t *sbufpool = NULL;
static mempool_t *rbufpool = NULL;
static int *sendcounts = NULL;
static int *displs = NULL;
static int sbufsize;
static int rbufsize;
static int root = 0;

int bench_iscatterv_init(nbctest_params_t *params)
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

int bench_iscatterv_free()
{
    mempool_free(sbufpool);
    mempool_free(rbufpool);
    free(sendcounts);
    free(displs);
    return MPIPERF_SUCCESS;
}

int bench_iscatterv_printinfo()
{
    printf("* Iscatterv\n"
           "  proto: MPI_Iscatterv(sbuf, sendcounts, displs, MPI_BYTE, \n"
           "                       rbuf, count, MPI_BYTE, %d, comm)\n"
           "  For each element: sendcounts[i] = count\n", root);
    return MPIPERF_SUCCESS;
}

int measure_iscatterv_blocking(nbctest_params_t *params,
		                       nbctest_result_t *result)
{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Iscatterv(mempool_alloc(sbufpool, sbufsize), sendcounts, displs,
                        MPI_BYTE, mempool_alloc(rbufpool, rbufsize), params->count,
                        MPI_BYTE, root, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
    	result->totaltime = endtime - starttime;
		return MEASURE_SUCCESS;
    }
#endif
    return MEASURE_FAILURE;
}

int measure_iscatterv_overlap(nbctest_params_t *params,
		                      nbctest_result_t *result)

{
#if MPICH2_NUMVERSION >= 10500002
    double starttime, endtime;
    int rc;
    static MPI_Request req;

    starttime = timeslot_startsync();
    result->inittime = hpctimer_wtime();
    rc = MPIX_Iscatterv(mempool_alloc(sbufpool, sbufsize), sendcounts, displs,
                        MPI_BYTE, mempool_alloc(rbufpool, rbufsize), params->count,
                        MPI_BYTE, root, params->comm, &req);
    result->inittime = hpctimer_wtime() - result->inittime;
    nbcbench_simulate_computing(params, &req, result);
    result->waittime = hpctimer_wtime();
    rc = MPI_Wait(&req, MPI_STATUS_IGNORE);
    result->waittime = hpctimer_wtime() - result->waittime;
    endtime = timeslot_stopsync();

    if ((rc == MPI_SUCCESS) && (starttime > 0.0) && (endtime > 0.0)) {
    	result->totaltime = endtime - starttime;
		return MEASURE_SUCCESS;
    }
    return MEASURE_FAILURE;
#endif
}
