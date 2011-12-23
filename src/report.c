/*
 * report.c:
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "report.h"
#include "mpiperf.h"
#include "bench.h"
#include "stats.h"
#include "util.h"
#include "seq.h"
#include "hpctimer.h"

#define OUTLIERS_FILTER_MIN 25
#define OUTLIERS_FILTER_MAX 25

static int confidence_level_type = STATS_CONFIDENCE_LEVEL_95;
static int confidence_level = 95;

enum {
    MPIPERF_REPORT_MSGTAG = 128,
    MPIPERF_REPORT_BUFSIZE = 1024
};

/* report_write_header: */
int report_write_header(bench_t *bench)
{
    int i, namelen;
    char procname[MPI_MAX_PROCESSOR_NAME];

    if (IS_MASTER_RANK) {
        /* TODO: Dump all options */
        printf("# mpiperf %d.%d.%d report\n", MPIPERF_VERSION_MAJOR,
                MPIPERF_VERSION_MINOR, MPIPERF_VERSION_PATCH);
        printf("# Command line: %s\n", mpiperf_cmdline);
        printf("# Benchmark: %s\n", bench->name);
        printf("# Variable parameter: %s\n", bench->paramname);
        printf("# Variable parameter range: min=%d max=%d\n",
               mpiperf_param_min, mpiperf_param_max);
        if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
            printf("# Test exit condition: %d successful runs\n",
                   mpiperf_nmeasures_max);
        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
            printf("# Test exit condition: relative stdandard error of measurements <= %.2f\n",
                   mpiperf_rse_max);
        }
        printf("# Minimal number of runs: %d\n", mpiperf_nruns_min);
        printf("# Maximal number of runs: %d\n", mpiperf_nruns_max);
        printf("# Cache defeat flag: %d\n", mpiperf_isflushcache);
        printf("# Timer: %s\n", mpiperf_timername);
        if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
            printf("# Time scale: seconds\n");
        } else {
            printf("# Time scale: microseconds\n");
        }
        if (mpiperf_synctype == MPIPERF_SYNC_TIME) {
            printf("# Synchronization method: synctime\n");
        } else {
            printf("# Synchronization method: nosync\n");
        }
        printf("# Master process: %d\n", mpiperf_master_rank);
        /* Print mapping of processes */
        printf("# Process mapping: \n");
        for (i = 0; i < mpiperf_commsize; i++) {
            if (i != mpiperf_master_rank) {
                MPI_Recv(procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i,
                         MPIPERF_REPORT_MSGTAG, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
            } else {
                MPI_Get_processor_name(procname, &namelen);
            }
            printf("#   %d:%s\n", i, procname);
        }

        printf("# Characteristics of measurements:\n");
        printf("#   %s - variable parameter\n", bench->paramname);
        printf("#   TRuns - total number of measurements (valid and invalid measurements)\n");
        printf("#   CRuns - number of correct measurements (only valid)\n");
        printf("#   FRuns - number of correct measurements after statistical analysis (removing of outliers)\n");
        printf("#   Mean - arithmetic mean of execution time (based on FRuns)\n");
        printf("#   RSE - relative standard error (StdErr / Mean)\n");
        printf("#   StdErr - standard error of the mean: StdDev / sqrt(FRuns)\n");
        printf("#   Min - minimal value\n");
        printf("#   Max - miximal value\n");
        printf("#   CL - confidence level: 90%%, 95%% or 99%%\n");
        printf("#   Err - error of measurements: t_student * StdErr\n");
        printf("#   CI LB - lower bound of confidence interval: Mean - Err\n");
        printf("#   CI UB - upper bound of confidence interval: Mean + Err\n");
        printf("#   RelErr - relative error of measurements: Err / Mean\n");
    } else {
        MPI_Get_processor_name(procname, &namelen);
        MPI_Send(procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                 mpiperf_master_rank, MPIPERF_REPORT_MSGTAG, MPI_COMM_WORLD);
    }

    return MPIPERF_SUCCESS;
}

/* report_write_colltest_synctime_header: */
int report_write_colltest_synctime_header(bench_t *bench)
{
    if (IS_MASTER_RANK) {
        printf("#\n");
        printf("# ------------------------------------------------------------------\n");
        printf("# %s time (maximum time off all processes)\n", bench->name);
        printf("# Variable parameter: %s\n", bench->paramname);
        printf("# Confidence level (CL): %d%%\n", confidence_level);
        printf("# ------------------------------------------------------------------\n");
        if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
            printf("# [Parameter] [TRuns] [CRuns] [FRuns] [Mean]       [RSE]      [StdErr]     [Min]        [Max]       [Err]        [CI LB]      [CI UB]      [RelErr]\n");
        } else {
            /* usec */
            printf("# [Parameter] [TRuns] [CRuns] [FRuns] [Mean]         [RSE]      [StdErr]       [Min]          [Max]         [Err]          [CI LB]        [CI UB]        [RelErr]\n");
        }
        printf("#\n");
    }

    return MPIPERF_SUCCESS;
}

/* report_write_colltest_nosync_header: */
int report_write_colltest_nosync_header(bench_t *bench)
{
    if (IS_MASTER_RANK) {
        printf("#\n");
        printf("# ------------------------------------------------------------------\n");
        printf("# %s time (maximum time off all processes)\n", bench->name);
        printf("# Variable parameter: %s\n", bench->paramname);
        printf("# ------------------------------------------------------------------\n");
        if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
            printf("# [Parameter] [TRuns] [Mean]\n");
        } else {
            /* usec */
            printf("# [Parameter] [TRuns] [Mean]\n");
        }
        printf("#\n");
    }

    return MPIPERF_SUCCESS;
}

/* report_write_pt2pttest_header: */
int report_write_pt2pttest_header(bench_t *bench)
{
    if (IS_MASTER_RANK) {
        printf("#\n");
        printf("# ------------------------------------------------------------------\n");
        printf("# %s time (time of the master process)\n", bench->name);
        printf("# Variable parameter: %s\n", bench->paramname);
        printf("# Confidence level (CL): %d%%\n", confidence_level);
        printf("# ------------------------------------------------------------------\n");
        if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
            printf("# [Parameter] [TRuns] [CRuns] [FRuns] [Mean]       [RSE]      [StdErr]     [Min]        [Max]       [Err]        [CI LB]      [CI UB]      [RelErr]\n");
        } else {
            /* usec */
            printf("# [Parameter] [TRuns] [CRuns] [FRuns] [Mean]         [RSE]      [StdErr]       [Min]          [Max]         [Err]          [CI LB]        [CI UB]        [RelErr]\n");
        }
        printf("#\n");
    }

    return MPIPERF_SUCCESS;
}

/*
 * report_write_colltest_synctime: Write execution times of the collective
 *                                 operation to the report.
 */
int report_write_colltest_synctime(bench_t *bench, double *exectime_local,
                                   int nruns, int ncorrectruns)
{
    double *exectime = NULL;
    double exectime_mean, exectime_stderr, exectime_rse, exectime_min,
           exectime_max, exectime_errrel;
    double exectime_ci_lb = 0.0, exectime_ci_ub = 0.0, exectime_err = 0.0;
    int nresultruns = 0;
    stats_sample_t *sample = NULL;
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
        fmt = "%-13d %-7d %-7d %-7d %-12.6f %-10.2f %-12.6f %-12.6f %-12.6f"
              "%-12.6f %-12.6f %-12.6f %-10.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "%-13d %-7d %-7d %-7d %-14.2f %-10.2f %-14.2f %-14.2f %-14.2f"
              "%-14.2f %-14.2f %-14.2f %-10.2f\n";
        timescale = 1E6;
    }

    if (ncorrectruns == 0 && IS_MASTER_RANK) {
        printf(fmt, bench->paramseq_getcurrent(bench), nruns, 0, 0, 0.0, 0.0,
               0.0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
        return MPIPERF_SUCCESS;
    }

    if (IS_MASTER_RANK) {
        exectime = xmalloc(sizeof(*exectime) * ncorrectruns);
    }

    /*
     * Gather maximum time on all processes for each repetition;
     * T_mem = O(ncorrectruns)
     */
    MPI_Reduce(exectime_local, exectime, ncorrectruns, MPI_DOUBLE, MPI_MAX,
               mpiperf_master_rank, bench->getcomm(bench));

    if (IS_MASTER_RANK) {
        if ( (sample = stats_sample_create()) == NULL) {
            free(exectime);
            return MPIPERF_FAILURE;
        }

        /* Remove outliers */
        nresultruns = ncorrectruns;
        if (mpiperf_statanalysis) {
            /* Remove 25% of minimal values and 25% of maximal values */
            nresultruns = stats_dataset_remove_extreme(exectime,
                                                       ncorrectruns,
                                                       OUTLIERS_FILTER_MIN,
                                                       OUTLIERS_FILTER_MAX);
        }

        stats_sample_add_dataset(sample, exectime, nresultruns);
        exectime_mean = stats_sample_mean(sample) * timescale;
        exectime_stderr = stats_sample_stderr(sample) * timescale;
        exectime_rse = stats_sample_stderr_rel(sample);
        exectime_min = stats_sample_min(sample) * timescale;
        exectime_max = stats_sample_max(sample) * timescale;

        /* Build confidence interval */
        stats_sample_confidence_interval(sample, confidence_level_type,
                                         &exectime_ci_lb, &exectime_ci_ub,
                                         &exectime_err);
        exectime_ci_lb *= timescale;
        exectime_ci_ub *= timescale;
        exectime_err *= timescale;
        exectime_errrel = (exectime_err > 0.0) ? exectime_err / exectime_mean : 0.0;

        printf(fmt, bench->paramseq_getcurrent(bench), nruns, ncorrectruns,
               nresultruns, exectime_mean, exectime_rse, exectime_stderr,
               exectime_min, exectime_max, exectime_err, exectime_ci_lb,
               exectime_ci_ub, exectime_errrel);

        stats_sample_free(sample);
    }

    if (IS_MASTER_RANK) {
        free(exectime);
    }

    return MPIPERF_SUCCESS;
}

/* report_write_colltest_nosync: */
int report_write_colltest_nosync(bench_t *bench, double exectime_local, int nruns)
{
    double exectime;
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
        fmt = "%-13d %-7d %-12.6f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "%-13d %-7d %-14.2f\n";
        timescale = 1E6;
    }

    if (nruns == 0 && IS_MASTER_RANK) {
        printf(fmt, bench->paramseq_getcurrent(bench), nruns, 0.0);
        return MPIPERF_SUCCESS;
    }

    /* Gather maximum time on all processes */
    MPI_Reduce(&exectime_local, &exectime, 1, MPI_DOUBLE, MPI_MAX,
               mpiperf_master_rank, bench->getcomm(bench));
	exectime *= timescale;

    if (IS_MASTER_RANK) {
        printf(fmt, bench->paramseq_getcurrent(bench), nruns, exectime);
    }

    return MPIPERF_SUCCESS;
}

/*
 * report_write_pt2pttest_exectime: Write execution times of the point-to-point
 *                                  operation to the report.
 */
int report_write_pt2pttest_exectime(bench_t *bench, double *exectime,
                                   int nruns, int ncorrectruns)
{
    double exectime_mean, exectime_stderr, exectime_rse, exectime_min,
           exectime_max, exectime_errrel;
    double exectime_ci_lb = 0.0, exectime_ci_ub = 0.0, exectime_err = 0.0;
    int nresultruns = 0;
    stats_sample_t *sample = NULL;
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
        fmt = "%-13d %-7d %-7d %-7d %-12.6f %-10.2f %-12.6f %-12.6f %-12.6f"
              "%-12.6f %-12.6f %-12.6f %-10.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "%-13d %-7d %-7d %-7d %-14.2f %-10.2f %-14.2f %-14.2f %-14.2f"
              "%-14.2f %-14.2f %-14.2f %-10.2f\n";
        timescale = 1E6;
    }

    if (!IS_MASTER_RANK) {
        return MPIPERF_SUCCESS;
    }

    if (ncorrectruns == 0) {
        printf(fmt, bench->paramseq_getcurrent(bench), nruns, 0, 0, 0.0, 0.0,
               0.0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        return MPIPERF_SUCCESS;
    }

    /* Remove outliers */
    nresultruns = ncorrectruns;
    if (mpiperf_statanalysis) {
        /* Remove 25% of minimal values and 25% of maximal values */
        nresultruns = stats_dataset_remove_extreme(exectime,
                                                   ncorrectruns,
                                                   OUTLIERS_FILTER_MIN,
                                                   OUTLIERS_FILTER_MAX);
    }

    if ( (sample = stats_sample_create()) == NULL) {
        return MPIPERF_FAILURE;
    }
    stats_sample_add_dataset(sample, exectime, nresultruns);
    exectime_mean = stats_sample_mean(sample) * timescale;
    exectime_stderr = stats_sample_stderr(sample) * timescale;
    exectime_rse = stats_sample_stderr_rel(sample);
    exectime_min = stats_sample_min(sample) * timescale;
    exectime_max = stats_sample_max(sample) * timescale;

    /* Build confidence interval */
    stats_sample_confidence_interval(sample, confidence_level_type,
                                     &exectime_ci_lb, &exectime_ci_ub,
                                     &exectime_err);
    exectime_ci_lb *= timescale;
    exectime_ci_ub *= timescale;
    exectime_err *= timescale;
    exectime_errrel = (exectime_err > 0.0) ? exectime_err / exectime_mean : 0.0;

    printf(fmt, bench->paramseq_getcurrent(bench), nruns, ncorrectruns,
           nresultruns, exectime_mean, exectime_rse, exectime_stderr,
           exectime_min, exectime_max, exectime_err, exectime_ci_lb,
           exectime_ci_ub, exectime_errrel);

    stats_sample_free(sample);
    return MPIPERF_SUCCESS;
}


/* report_write_process_stat_synctime: Prints report for each process. */
int report_write_process_stat_synctime(bench_t *bench, stats_sample_t **procstat)
{
    int i, n, paramval;
    enum { NSTATS_PARAMS = 9 };
    double statsparams[NSTATS_PARAMS], *allstats = NULL;
    double lb, ub, err, errrel;
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
        fmt = "%-13d %-7d %-7d %-12.6f %-10.2f %-12.6f %-12.6f %-12.6f"
              "%-12.6f %-12.6f %-12.6f %-10.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "%-13d %-7d %-7d %-14.2f %-10.2f %-14.2f %-14.2f %-14.2f"
              "%-14.2f %-14.2f %-14.2f %-10.2f\n";
        timescale = 1E6;
    }

    if (IS_MASTER_RANK) {
        allstats = xmalloc(sizeof(*allstats) * NSTATS_PARAMS * mpiperf_commsize);

        printf("\n");
        printf("# ------------------------------------------------------------------\n");
        printf("# %s time at a process\n", bench->name);
        printf("# Variable parameter: %s\n", bench->paramname);
        printf("# Confidence level (CL): %d%%\n", confidence_level);
        printf("# ------------------------------------------------------------------\n");
        if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
            printf("# [Parameter] [Proc]  [CRuns] [Mean]       [RSE]      [StdErr]     [Min]        [Max]       [Err]        [CI LB]      [CI UB]      [RelErr]\n");
        } else {
            /* usec */
            printf("# [Parameter] [Proc]  [CRuns] [Mean]         [RSE]      [StdErr]       [Min]          [Max]         [Err]          [CI LB]        [CI UB]        [RelErr]\n");
        }
        printf("#\n");
    }

    /* Iterate over all parameter values */
    bench->paramseq_reset(bench);
    n = 0;
    for (paramval = bench->paramseq_getcurrent(bench);
         paramval > 0;
         paramval = bench->paramseq_getnext(bench))
    {
        if (procstat[n]) {
            statsparams[0] = stats_sample_mean(procstat[n]) * timescale;
            statsparams[1] = stats_sample_stderr(procstat[n]) * timescale;
            statsparams[2] = stats_sample_stderr_rel(procstat[n]);
            statsparams[3] = stats_sample_min(procstat[n]) * timescale;
            statsparams[4] = stats_sample_max(procstat[n]) * timescale;
            statsparams[5] = (double)stats_sample_size(procstat[n]);

            stats_sample_confidence_interval(procstat[n], confidence_level_type,
                                             &lb, &ub, &err);
            statsparams[6] = lb * timescale;
            statsparams[7] = ub * timescale;
            statsparams[8] = err * timescale;
        } else {
            /* For this parameter value process didn't participate in test */
            statsparams[0] = 0.0;
            statsparams[1] = 0.0;
            statsparams[2] = 0.0;
            statsparams[3] = 0.0;
            statsparams[4] = 0.0;
            statsparams[5] = 0.0;
            statsparams[6] = 0.0;
            statsparams[7] = 0.0;
            statsparams[8] = 0.0;
        }
        n++;

        /* Gather statistics: T_mem  = O(n) */
        MPI_Gather(statsparams, NSTATS_PARAMS, MPI_DOUBLE, allstats,
                   NSTATS_PARAMS, MPI_DOUBLE, mpiperf_master_rank,
                   MPI_COMM_WORLD);

        if (IS_MASTER_RANK) {
            for (i = 0; i < bench->getcommsize(bench); i++) {
                if ((int)allstats[i * NSTATS_PARAMS + 5] > 0) {
                    /* Sample is not empty: samplesize > 0 */
                    errrel = 0.0;
                    if (allstats[i * NSTATS_PARAMS + 8] > 0.0) {
                        /* err > 0 */
                        errrel = allstats[i * NSTATS_PARAMS + 8] /
                                 allstats[i * NSTATS_PARAMS + 0];
                    }
                    printf(fmt, paramval, i,
                           (int)allstats[i * NSTATS_PARAMS + 5], /* Runs */
                           allstats[i * NSTATS_PARAMS + 0],      /* Mean */
                           allstats[i * NSTATS_PARAMS + 2],      /* RSE */
                           allstats[i * NSTATS_PARAMS + 1],      /* StdErr */
                           allstats[i * NSTATS_PARAMS + 3],      /* Min */
                           allstats[i * NSTATS_PARAMS + 4],      /* Max */
                           allstats[i * NSTATS_PARAMS + 8],      /* Err */
                           allstats[i * NSTATS_PARAMS + 6],      /* CI LB */
                           allstats[i * NSTATS_PARAMS + 7],      /* CI UB */
                           errrel                                /* Err / Mean */
                          );

                } else {
                    printf(fmt, paramval, i,
                           0,   /* Runs */
                           0.0, /* Mean */
                           0.0, /* RSE */
                           0.0, /* StdErr */
                           0.0, /* Min */
                           0.0, /* Max */
                           0.0, /* Err */
                           0.0, /* CI LB */
                           0.0, /* CI UB */
                           0.0  /* Err / Mean */
                          );
                }
            }
        }
    }

    if (IS_MASTER_RANK) {
        free(allstats);
    }

    return MPIPERF_SUCCESS;
}

/* report_write_process_stat_nosync: Prints report for each process. */
int report_write_process_stat_nosync(bench_t *bench, stats_sample_t **procstat)
{
    int i, n, paramval;
    double exectime, *allstats = NULL;
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
        fmt = "%-13d %-7d %-7d %-12.6f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "%-13d %-7d %-7d %-14.2f\n";
        timescale = 1E6;
    }

    if (IS_MASTER_RANK) {
        allstats = xmalloc(sizeof(*allstats) * mpiperf_commsize);

        printf("\n");
        printf("# ------------------------------------------------------------------\n");
        printf("# %s time at a process\n", bench->name);
        printf("# Variable parameter: %s\n", bench->paramname);
        printf("# ------------------------------------------------------------------\n");
        if (mpiperf_timescale == MPIPERF_TIMESCALE_SEC) {
            printf("# [Parameter] [Proc]  [TRuns] [Mean]\n");
        } else {
            /* usec */
            printf("# [Parameter] [Proc]  [TRuns] [Mean]\n");
        }
        printf("#\n");
    }

    /* Iterate over all parameter values */
    bench->paramseq_reset(bench);
    n = 0;
    for (paramval = bench->paramseq_getcurrent(bench);
         paramval > 0;
         paramval = bench->paramseq_getnext(bench))
    {
        if (procstat[n]) {
            exectime = stats_sample_mean(procstat[n]) * timescale;
        } else {
            /* For this parameter value process didn't participate in test */
            exectime = 0.0;
        }
        n++;

        /* Gather statistics: T_mem  = O(n) */
        MPI_Gather(&exectime, 1, MPI_DOUBLE, allstats, 1, MPI_DOUBLE,
        		   mpiperf_master_rank, MPI_COMM_WORLD);

        if (IS_MASTER_RANK) {
            for (i = 0; i < bench->getcommsize(bench); i++) {
	            printf(fmt, paramval, i, mpiperf_nruns_max, allstats[i]);
            }
        }
    }

    if (IS_MASTER_RANK) {
        free(allstats);
    }

    return MPIPERF_SUCCESS;
}

/* report_printf: Prints an message to the report. */
void report_printf(const char *format, ...)
{
    va_list ap;
    static char buf[MPIPERF_REPORT_BUFSIZE];

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    printf("%s", buf);
}

