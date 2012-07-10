/*
 * timeslot.h: Timeslots module.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#ifndef TIMESLOT_H
#define TIMESLOT_H

#define TIMESLOT_TIME_INVALID -1.0

#include <mpi.h>

/* timeslot_initialize: */
int timeslot_initialize();

/* timeslot_initialize_test: */
int timeslot_initialize_test(MPI_Comm comm);

/* timeslot_setlen: Set length of the timeslots in seconds. */
void timeslot_set_length(double length);

/* timeslot_set_starttime: Set start time. */
double timeslot_set_starttime(MPI_Comm comm);

/*
 * timeslot_startsync: Wait for the next timeslot and returns its start time
 *                     in seconds.
 */
double timeslot_startsync();

/*
 * timeslot_stopsync: Returns timestamp in seconds or TIMESLOT_TIME_INVALID
 *                    if current timeslot have finished.
 */
double timeslot_stopsync();

/* timeslot_finalize: */
void timeslot_finalize();

#endif /* TIMESLOT_H */
