/*
 * mpiperf.h: MPI Performance Benchmark.
 *
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#ifndef MPIPERF_H
#define MPIPERF_H

#include <stdio.h>
#include "version.h"

#define NELEMS(v) (sizeof(v) / sizeof((v)[0]))
#define IS_MASTER_RANK (mpiperf_rank == mpiperf_master_rank)

#define TEST_SLOTLEN_SCALE 1.1
#define OUTLIERS_FILTER_MIN 25
#define OUTLIERS_FILTER_MAX 25

enum ReturnCode {
    MPIPERF_SUCCESS = 0,
    MPIPERF_FAILURE = 1
};

enum ParamStepType {
	STEP_TYPE_INC = 0,
	STEP_TYPE_MUL = 1
};

enum ExitConditionType {
    TEST_EXIT_COND_NRUNS = 0,
    TEST_EXIT_COND_STDERR = 1
};

enum TimeScaleType {
    TIMESCALE_SEC = 0,
    TIMESCALE_USEC = 1
};

enum SyncTimeType {
    SYNC_TIME = 0,
    SYNC_NONE = 1
};

enum MeasureReturnCodes {
	MEASURE_SUCCESS = 0,
	MEASURE_FAILURE = 1,
	MEASURE_STARTED_LATE = 3,
	MEASURE_TIME_TOOLONG = 4
};

enum StageConsts {
    TEST_REALLOC_GROWSTEP = 2,
    TEST_STAGE_NRUNS_INIT = 4,
    TEST_STAGE_NRUNS = 8
};

enum NBCBenchMode {
	NBCBENCH_BLOCKING = 0,
	NBCBENCH_OVERLAP = 1
};

enum ReportConsts {
    REPORT_MSGTAG = 128,
    REPORT_BUFSIZE = 1024
};

extern char *mpiperf_progname;    /* mpiperf application name */

extern int mpiperf_rank;          /* Rank of current process in MPI_COMM_WORLD */
extern int mpiperf_commsize;      /* Size of MPI_COMM_WORLD */
extern int mpiperf_master_rank;   /* Process which generates report */
extern char *mpiperf_cmdline;

extern int mpiperf_nprocs_min;
extern int mpiperf_nprocs_max;
extern int mpiperf_nprocs_step_type;
extern int mpiperf_nprocs_step;

extern int mpiperf_count_min;
extern int mpiperf_count_max;
extern int mpiperf_count_step_type;
extern int mpiperf_count_step;

extern int mpiperf_perprocreport;
extern char *mpiperf_repfile;
extern FILE *mpiperf_repstream;

extern int mpiperf_isflushcache;
extern int mpiperf_statanalysis;
extern int mpiperf_timescale;
extern int mpiperf_synctype;
extern char *mpiperf_timername;

extern int mpiperf_test_exit_cond;
extern int mpiperf_nmeasures_max;
extern double mpiperf_rse_max;
extern int mpiperf_nruns_min;
extern int mpiperf_nruns_max;

extern char *mpiperf_logfile;
extern int mpiperf_logmaster_only;

extern int mpiperf_confidence_level_type;
extern int mpiperf_confidence_level;

extern int mpiperf_nbcbench_mode;
extern int mpiperf_comptime_niters;

extern char *mpiperf_benchname;

extern int mpiperf_is_measure_started;

void mpiperf_initialize();
void mpiperf_finalize();

#endif /* MPIPERF_H */
