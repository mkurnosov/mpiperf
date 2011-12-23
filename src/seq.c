/*
 * seq.c: Sequence of parameter values.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <mpi.h>

#include "seq.h"
#include "mpiperf.h"

static int valmin;
static int valmax;
static int valstep;
static int val_steptype;
static int valcurrent;
static MPI_Comm curcomm;
static int curcomm_rank;
static int curcomm_size;

static MPI_Comm createcomm(int size);

int intseq_initialize(bench_t *bench, int min, int max, int step, int steptype)
{
    valmin = min;
    valcurrent = valmin;
    valmax = max;
    valstep = step;
    val_steptype = steptype;
    return MPIPERF_SUCCESS;
}
                 
int intseq_finalize(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

/* 
 * intseq_getnext: Returns next value in the sequence.
 *                 Returns -1 if current value exceeds maximal value.
 */
int intseq_getnext(bench_t *bench)
{
    if (val_steptype == SEQ_STEP_INC) {
        valcurrent += valstep;
    } else if (val_steptype == SEQ_STEP_MUL) {
        valcurrent *= valstep;
    }
    if (valcurrent > valmax)
        return -1;
    return valcurrent;
}

/* intseq_getsize: Returns number of values in the sequence. */
int intseq_getsize(bench_t *bench)
{
    int val, count = 0;
    
    /* TODO: compute count via sum of progression */
        
    if (val_steptype == SEQ_STEP_INC) {
        for (val = valmin; val <= valmax; val += valstep) {
            count++;
        }
    } else if (val_steptype == SEQ_STEP_MUL) {
        for (val = valmin; val <= valmax; val *= valstep) {
            count++;
        }
    }
    return count;
}

/* intseq_getcurrent: */
int intseq_getcurrent(bench_t *bench)
{
    return valcurrent;
}

/* intseq_reset: Resets current value to the first. */
void intseq_reset(bench_t *bench)
{
    valcurrent = valmin;
}

/* commseq_initialize: */
int commseq_initialize(bench_t *bench, int min, int max, int step, int steptype)
{
    valmin = min;
    valcurrent = valmin;
    valmax = max;
    valstep = step;
    val_steptype = steptype;
    curcomm = createcomm(valcurrent);
    return MPIPERF_SUCCESS;
}

int commseq_finalize(bench_t *bench)
{
    return MPIPERF_SUCCESS;
}

int commseq_getnext(bench_t *bench)
{
    if (val_steptype == SEQ_STEP_INC) {
        valcurrent += valstep;
    } else if (val_steptype == SEQ_STEP_MUL) {
        valcurrent *= valstep;
    }

    /* Free current communicator */
    if (curcomm != MPI_COMM_NULL) {
        MPI_Comm_free(&curcomm);
        curcomm = MPI_COMM_NULL;
    }

    if (valcurrent > valmax) {
        return -1;
    }
    /* Create new communicator */
    curcomm = createcomm(valcurrent);
    return valcurrent;
}

int commseq_getsize(bench_t *bench)
{
    int val, count = 0;

    /* TODO: compute count via sum of progression */

    if (val_steptype == SEQ_STEP_INC) {
        for (val = valmin; val <= valmax; val += valstep) {
            count++;
        }
    } else if (val_steptype == SEQ_STEP_MUL) {
        for (val = valmin; val <= valmax; val *= valstep) {
            count++;
        }
    }
    return count;
}

int commseq_getcurrent(bench_t *bench)
{
    return valcurrent;
}

void commseq_reset(bench_t *bench)
{
    valcurrent = valmin;
    curcomm = createcomm(valcurrent);
}

MPI_Comm commseq_getcomm(bench_t *bench)
{
    return curcomm;
}

int commseq_getcommsize(bench_t *bench)
{
    return curcomm_size;
}

int commseq_getcommrank(bench_t *bench)
{
    return curcomm_rank;
}

static MPI_Comm createcomm(int size)
{
    MPI_Comm newcomm;
    MPI_Comm_split(MPI_COMM_WORLD, (mpiperf_rank < size) ? 0 : MPI_UNDEFINED,
                   mpiperf_rank, &newcomm);
    if (newcomm != MPI_COMM_NULL) {
        MPI_Comm_size(newcomm, &curcomm_size);
        MPI_Comm_rank(newcomm, &curcomm_rank);
    }
    return newcomm;
}


