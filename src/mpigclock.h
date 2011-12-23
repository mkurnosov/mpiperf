/*
 * mpigclock.h: MPI clock synchronization library.
 *
 * Copyright (C) 2010-2011 Mikhail Kurnosov <mkurnosov@gmail.com>
 */

#ifndef MPIGCLOCK_H
#define MPIGCLOCK_H

#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return codes */
enum {
    MPIGCLOCK_SUCCESS = 0,
    MPIGCLOCK_FAILURE = 1
};

/* Clock synchronization algorithms */
enum SyncAlg {
    MPIGCLOCK_SYNC_LINEAR = 0,
    MPIGCLOCK_SYNC_RING = 1
};

/* mpigclock_sync: Synchronizes clocks of processes with clock of the root. */
int mpigclock_sync(MPI_Comm comm, int root, int syncalg);

/*
 * mpigclock_offset: Returns local clock's offset relative
 *                   to clock of the root.
 */
double mpigclock_offset();

#ifdef __cplusplus
}
#endif

#endif /* MPIGCLOCK_H */


