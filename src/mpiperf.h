/*
 * mpiperf.h: MPI Performance Benchmark.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef MPIPERF_H
#define MPIPERF_H

#include "bench.h"

#define MPIPERF_VERSION_MAJOR 0
#define MPIPERF_VERSION_MINOR 0
#define MPIPERF_VERSION_PATCH 1

#define IS_MASTER_RANK (mpiperf_rank == mpiperf_master_rank)

enum {
    MPIPERF_SUCCESS = 0,
    MPIPERF_FAILURE = 1
};

enum {
    MPIPERF_TIMESCALE_SEC = 0,
    MPIPERF_TIMESCALE_USEC = 1
};

enum {
    MPIPERF_SYNC_TIME = 0,
    MPIPERF_SYNC_NONE = 1
};

extern const char *mpiperf_progname;    /* mpiperf application name */

extern int mpiperf_rank;          /* Rank of current process in MPI_COMM_WORLD */
extern int mpiperf_commsize;      /* Size of MPI_COMM_WORLD */
extern int mpiperf_master_rank;   /* Process which generates report */

extern char *mpiperf_cmdline;
extern int mpiperf_isflushcache;  /* Cache defeat flag */
extern int mpiperf_genprocreport; /* Generate per process report flag */
extern int mpiperf_statanalysis;  /* Postprocess statistical analysis flag */
extern int mpiperf_timescale;
extern int mpiperf_synctype;

extern char *mpiperf_timername;

extern int mpiperf_param_step_type;
extern int mpiperf_param_min;
extern int mpiperf_param_max;
extern int mpiperf_param_step;

extern int mpiperf_test_exit_cond;
extern int mpiperf_nmeasures_max;
extern double mpiperf_rse_max;
extern int mpiperf_nruns_min;
extern int mpiperf_nruns_max;

void mpiperf_initialize();
void mpiperf_finalize();

#endif /* MPIPERF_H */
