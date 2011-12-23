/*
 * mpigclock.c: MPI clock synchronization.
 *
 * Copyright (C) 2011 Mikhail Kurnosov <mkurnosov@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "mpigclock.h"
#include "hpctimer.h"

#define INVALIDTIME -1.0
#define MPIGCLOCK_RTTMIN_NOTCHANGED_MAX 100
#define MPIGCLOCK_MSGTAG 128

static double mpigclock_local_offset;  /* Offset relative to root clock */

static int mpigclock_sync_linear(MPI_Comm comm, int root);
static int mpigclock_sync_ring(MPI_Comm comm, int root);
static double mpigclock_measure_offset_adaptive(MPI_Comm comm, int root, int peer);

/* mpigclock_sync: Synchronizes clocks of processes with clock of the root. */
int mpigclock_sync(MPI_Comm comm, int root, int syncalg)
{
    if (syncalg == MPIGCLOCK_SYNC_LINEAR) {
        return mpigclock_sync_linear(comm, root);
    } else if (syncalg == MPIGCLOCK_SYNC_RING) {
        return mpigclock_sync_ring(comm, root);
    }
    return MPIGCLOCK_FAILURE;
}

/*
 * mpigclock_offset: Returns offset of local clock relative to clock of the root.
 */
double mpigclock_offset()
{
    return mpigclock_local_offset;
}

/*
 * mpigclock_sync_linear: Clock synchronization algorithm with O(n) steps.
 */
static int mpigclock_sync_linear(MPI_Comm comm, int root)
{
    int i, rank, commsize;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commsize);

    if (commsize < 2) {
        mpigclock_local_offset = 0.0;
        return MPIGCLOCK_SUCCESS;
    }

    for (i = 1; i < commsize; i++) {
        MPI_Barrier(comm);
        if (rank == root || rank == i) {
            mpigclock_local_offset = mpigclock_measure_offset_adaptive(comm, root, i);
        }
    }
    return MPIGCLOCK_SUCCESS;
}

/*
 * mpigclock_sync_ring: Parallel algorithm of clock synchronization.
 */
static int mpigclock_sync_ring(MPI_Comm comm, int root)
{
    int i, rank, commsize, prev, next;
    double sum, offset = 0.0;
    double *offsets = NULL;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commsize);

    if (commsize < 2) {
        mpigclock_local_offset = 0.0;
        return MPIGCLOCK_SUCCESS;
    }

    prev = (rank - 1 + commsize) % commsize;
    next = (rank + 1) % commsize;

    MPI_Barrier(comm);

    if (rank % 2 == 0) {
        /* Server for next */
        mpigclock_measure_offset_adaptive(comm, rank, next);
        /* Client for prev */
        offset = mpigclock_measure_offset_adaptive(comm, prev, rank);
    } else {
        /* Client for prev */
        offset = mpigclock_measure_offset_adaptive(comm, prev, rank);
        /* Server for next */
        mpigclock_measure_offset_adaptive(comm, rank, next);
    }

    if (rank == root) {
        if ( (offsets = malloc(sizeof(*offsets) * commsize)) == NULL) {
            return MPIGCLOCK_FAILURE;
        }
    }

    MPI_Gather(&offset, 1, MPI_DOUBLE, offsets, 1, MPI_DOUBLE, root, comm);
    if (rank == root) {
        /* Compute relative offset */
        sum = offsets[root] = 0.0;
        for (i = (root + 1) % commsize; i != root; i = (i + 1) % commsize) {
            offsets[i] += sum;
            sum = offsets[i];
        }
    }
    MPI_Scatter(offsets, 1, MPI_DOUBLE, &mpigclock_local_offset, 1,
                MPI_DOUBLE, root, comm);

    if (rank == root) {
        free(offsets);
    }

    return MPIGCLOCK_SUCCESS;
}

/* mpigclock_measure_offset_adaptive: Measures clock's offset of peer. */
static double mpigclock_measure_offset_adaptive(MPI_Comm comm, int root, int peer)
{
    int rank, commsize, rttmin_notchanged = 0;
    double starttime, stoptime, peertime, rtt, rttmin = 1E12,
           invalidtime = INVALIDTIME, offset;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &commsize);

    offset = 0.0;
    for (;;) {
        if (rank != root) {
            /* Peer process */
            starttime = hpctimer_wtime();
            MPI_Send(&starttime, 1, MPI_DOUBLE, root, MPIGCLOCK_MSGTAG, comm);
            MPI_Recv(&peertime, 1, MPI_DOUBLE, root, MPIGCLOCK_MSGTAG, comm,
                     MPI_STATUS_IGNORE);
            stoptime = hpctimer_wtime();
            rtt = stoptime - starttime;

            if (rtt < rttmin) {
                rttmin = rtt;
                rttmin_notchanged = 0;
                offset =  peertime - rtt / 2.0 - starttime;
            } else {
                if (++rttmin_notchanged == MPIGCLOCK_RTTMIN_NOTCHANGED_MAX) {
                    MPI_Send(&invalidtime, 1, MPI_DOUBLE, root, MPIGCLOCK_MSGTAG,
                             comm);
                    break;
                }
            }
        } else {
            /* Root process */
            MPI_Recv(&starttime, 1, MPI_DOUBLE, peer, MPIGCLOCK_MSGTAG, comm,
                     MPI_STATUS_IGNORE);
            peertime = hpctimer_wtime();
            if (starttime < 0.0) {
                break;
            }
            MPI_Send(&peertime, 1, MPI_DOUBLE, peer, MPIGCLOCK_MSGTAG, comm);
        }
    } /* for */
    return offset;
}
