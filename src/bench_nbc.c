/*
 * bench_nbc.c: Functions for benchmarking MPI non-blocking collective routines.
 *
 * Copyright (C) 2011-2012 Mikhail Kurnosov
 */

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <mpi.h>

#include "bench_nbc.h"
#include "mpiperf.h"
#include "stat.h"
#include "timeslot.h"
#include "report.h"
#include "logger.h"
#include "util.h"
#include "hpctimer.h"
#include "bench_nbc_tab.h"

#define TEST_SLOTLEN_SCALE 1.1

#define BLOCKINGTIME_RSE_MAX 0.05
#define BLOCKINGTIME_NCORRECTRUNS_MIN 5
#define BLOCKINGTIME_NCORRECTRUNS_MAX 30

enum {
    COMPTIME_SCALE = 2
};

enum {
    TESTS_PER_COMPTIME = 10
};

/* run_nbcbench: Runs benchmark for NBC operation */
int run_nbcbench(nbcbench_t *bench)
{
    double benchtime;
    nbctest_params_t params;

    if (mpiperf_synctype == SYNC_NONE) {
        exit_error("nosync mode is not supported for NBC operations");
    }

    benchtime = hpctimer_wtime();
    timeslot_initialize();

    report_write_header();
    report_write_nbcbench_header(bench);
    if (mpiperf_perprocreport)
        report_write_nbcbench_procstat_header(bench);

    /* For each communicator size */
    for (params.nprocs = mpiperf_nprocs_min;
         params.nprocs <= mpiperf_nprocs_max; )
    {
        params.comm = createcomm(MPI_COMM_WORLD, params.nprocs);

        /* For each data size (count) */
        for (params.count = mpiperf_count_min;
             params.count <= mpiperf_count_max; )
        {
            /* Test NBC collective operation for given nprocs and count */
            if (mpiperf_nbcbench_mode == NBCBENCH_OVERLAP)
                run_nbcbench_overlap(bench, &params);
            else
                run_nbcbench_blocking(bench, &params);

            if (mpiperf_count_step_type == STEP_TYPE_MUL) {
                params.count *= mpiperf_count_step;
            } else {
                params.count += mpiperf_count_step;
            }
        }
        if (params.comm != MPI_COMM_NULL)
            MPI_Comm_free(&params.comm);

        if (mpiperf_nprocs_step_type == STEP_TYPE_MUL) {
            params.nprocs *= mpiperf_nprocs_step;
        } else {
            params.nprocs += mpiperf_nprocs_step;
        }
    }

    timeslot_finalize();
    MPI_Barrier(MPI_COMM_WORLD);
    benchtime = hpctimer_wtime() - benchtime;
    if (IS_MASTER_RANK) {
        report_printf("# Elapsed time: %.6f sec.\n", benchtime);
    }
    return MPIPERF_SUCCESS;
}

/*
 * run_nbcbench_overlap: Measures execution time of the NBC operation
 *                       in overlap mode.
 */
int run_nbcbench_overlap(nbcbench_t *bench, nbctest_params_t *params)
{
    double blockingtime, blockingtime_local;
    double comptime_min, comptime_max, comptime_step;
    int nruns, ncorrectruns;
    stat_sample_t *inittimestat, *waittimestat, *comptimestat, *totaltimestat,
                  *overlapstat;

    MPI_Barrier(MPI_COMM_WORLD);
    logger_log("Test (nprocs = %d, count = %d) is started",
               params->nprocs, params->count);

    if (params->comm != MPI_COMM_NULL) {
        /* This process participates in measures */

        inittimestat = stat_sample_create();
        waittimestat = stat_sample_create();
        comptimestat = stat_sample_create();
        totaltimestat = stat_sample_create();
        overlapstat = stat_sample_create();
        if (inittimestat == NULL || waittimestat == NULL || comptimestat == NULL
            || totaltimestat == NULL || overlapstat == NULL)
        {
            exit_error("Can't allocate memory for statistic");
        }

        /* Measure NBC operation time in blocking mode */
        nbcbench_measure_blocking_time(bench, params, &blockingtime,
                                       &blockingtime_local);
        logger_log("Blocking time global: %.6f sec, local: %.6f sec",
                   blockingtime, blockingtime_local);

        if (blockingtime < 1E-6) {
            blockingtime = 1E-9;
            logger_log("Global blocking time is too small: set it to %.9f",
                       blockingtime);
        }

        if (blockingtime_local < 1E-6) {
            blockingtime_local = 1E-9;
            logger_log("Local blocking time is too small: set it to %.9f",
                       blockingtime_local);
        }

        if (mpiperf_comptime_niters == 1) {
            comptime_min = blockingtime * COMPTIME_SCALE;
            comptime_max = blockingtime * COMPTIME_SCALE + 1.0;
            comptime_step = comptime_max;
        } else {
            comptime_min = 0.0;
            comptime_max = blockingtime * COMPTIME_SCALE;
            comptime_step = comptime_max / mpiperf_comptime_niters;
        }

        /* For each comptime */
        for (params->comptime = comptime_min; params->comptime <= comptime_max;
             params->comptime += comptime_step)
        {
            stat_sample_clean(inittimestat);
            stat_sample_clean(waittimestat);
            stat_sample_clean(comptimestat);
            stat_sample_clean(totaltimestat);
            stat_sample_clean(overlapstat);

            run_nbcbench_overlap_test(bench, params, blockingtime_local,
                                      &nruns, &ncorrectruns, inittimestat,
                                      waittimestat, comptimestat, totaltimestat,
                                      overlapstat);

            report_write_nbcbench_overlap(bench, params, nruns,
                                          ncorrectruns, blockingtime,
                                          inittimestat, waittimestat,
                                          comptimestat, totaltimestat,
                                          overlapstat);

            if (mpiperf_perprocreport) {
                report_write_nbcbench_procstat_overlap(bench, params, nruns,
                                                       ncorrectruns,
                                                       blockingtime_local,
                                                       inittimestat,
                                                       waittimestat,
                                                       comptimestat,
                                                       totaltimestat,
                                                       overlapstat);
            }
        }
        stat_sample_free(inittimestat);
        stat_sample_free(waittimestat);
        stat_sample_free(comptimestat);
        stat_sample_free(totaltimestat);
        stat_sample_free(overlapstat);
    }
    return MPIPERF_SUCCESS;
}

/*
 * run_nbcbench_overlap_test: Measures execution time of the NBC operation
 *                            in current process.
 *
 * We use window-based approach for benchmarking NBC.
 */
int run_nbcbench_overlap_test(nbcbench_t *bench,
                              nbctest_params_t *params,
                              double blockingtime,
                              int *nruns, int *ncorrectruns,
                              stat_sample_t *inittimestat,
                              stat_sample_t *waittimestat,
                              stat_sample_t *comptimestat,
                              stat_sample_t *totaltimestat,
                              stat_sample_t *overlapstat)
{
    int i, stage, stage_nruns, nerrors;
    int *stagerc = NULL, *stagerc_reduced = NULL;
    double *stage_exectime = NULL, *exectime_reduced = NULL;
    nbctest_result_t *stage_results = NULL;
    double stagetime, stagetime_max, slotlen;
    stat_sample_t *timestat;

    if ( (timestat = stat_sample_create()) == NULL) {
        exit_error("No enough memory");
    }
    stage_exectime = xrealloc(stage_exectime, sizeof(*stage_exectime) *
                              TEST_STAGE_NRUNS);
    exectime_reduced = xrealloc(exectime_reduced, sizeof(*exectime_reduced) *
                                TEST_STAGE_NRUNS);
    stage_results = xrealloc(stage_results, sizeof(*stage_results) *
                             TEST_STAGE_NRUNS);
    stagerc = xrealloc(stagerc, sizeof(*stagerc) * TEST_STAGE_NRUNS);
    stagerc_reduced = xrealloc(stagerc_reduced, sizeof(*stagerc_reduced) *
                               TEST_STAGE_NRUNS);

    if (bench->init)
        bench->init(params);

    *nruns = 0;
    *ncorrectruns = 0;
    stage_nruns = TEST_STAGE_NRUNS_INIT;

    slotlen = 0.0;
    timeslot_initialize_test(params->comm);

    for (stage = -1; ; stage++) {

        logger_flush();
        timeslot_set_length(slotlen);
        timeslot_set_starttime(params->comm);

        /* Run measurements */
        mpiperf_is_measure_started = 1;
        stagetime = hpctimer_wtime();

        for (i = 0; i < stage_nruns; i++) {
            stagerc[i] = bench->overlapop(params, &stage_results[i]);
        }

        stagetime = hpctimer_wtime() - stagetime;
        mpiperf_is_measure_started = 0;

        MPI_Allreduce(&stagetime, &stagetime_max, 1, MPI_DOUBLE, MPI_MAX,
                      params->comm);

        if (stage == -1) {
            /* It was a warmup stage */
            slotlen = stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE;
            stage_nruns = TEST_STAGE_NRUNS;
            logger_log("Warmup stage is finished: %d runs, slotlen = %.6f",
                       TEST_STAGE_NRUNS_INIT, slotlen);
            continue;
        }
        (*nruns) = (*nruns) + stage_nruns;

        /* Reduce exectime */
        for (i = 0; i < stage_nruns; i++) {
            stage_exectime[i] = stage_results[i].totaltime;
        }
        MPI_Allreduce(stage_exectime, exectime_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, params->comm);

        /* Gather return codes of previous stage */
        MPI_Allreduce(stagerc, stagerc_reduced, stage_nruns,
                      MPI_INT, MPI_MAX, params->comm);

        nerrors = 0;
        for (i = 0; i < stage_nruns; i++) {
            if (stagerc_reduced[i] == MEASURE_SUCCESS) {
                (*ncorrectruns)++;
                stat_sample_add(timestat, exectime_reduced[i]);
                stat_sample_add(inittimestat, stage_results[i].inittime);
                stat_sample_add(waittimestat, stage_results[i].waittime);
                stat_sample_add(comptimestat, stage_results[i].realcomptime);
                stat_sample_add(totaltimestat, stage_results[i].totaltime);
                stat_sample_add(overlapstat,
                                1.0 - (stage_results[i].totaltime - params->comptime) / blockingtime);
                logger_log("Overlap measure (comptime: %.6f; stage %d; run %d): "
                           "totaltime = %.6f; ntests = %d; ",
                           params->comptime, stage, i, stage_results[i].totaltime,
                           stage_results[i].ntests);
            } else {
                /*
                 * Some of processes at the measure #i
                 * was started after established time or terminated
                 * after time slot deadline or collop was failed.
                 */
                nerrors++;
            }
        }

        logger_log("Stage %d is finished: %d runs, %d invalid runs, RSE = %.2f",
                   stage, stage_nruns, nerrors, stat_sample_stderr_rel(timestat));

        /* Check exit condition */
        if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
            if ((*ncorrectruns >= mpiperf_nmeasures_max ||
                *nruns >= mpiperf_nruns_max) && (*nruns >= mpiperf_nruns_min))
            {
                break;
            }
        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
            if (((stat_sample_stderr_rel(timestat) <= mpiperf_rse_max) &&
                 *ncorrectruns >= mpiperf_nruns_min) || (*nruns >= mpiperf_nruns_max))
            {
                break;
            }
        }

        /* Adjust timeslot parameters */
        if (nerrors > stage_nruns / 4.0) {
            slotlen = stat_fmax2(2.0 * slotlen,
                                 stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE);
            logger_log("Corrected timeslot length: %.6f", slotlen);
        }

    } /* stages */

    logger_log("Test is finished: %d runs, %d correct runs, RSE = %.2f",
               *nruns, *ncorrectruns, stat_sample_stderr_rel(timestat));

    if (bench->free)
        bench->free();

    free(stage_exectime);
    free(exectime_reduced);
    free(stage_results);
    free(stagerc_reduced);
    free(stagerc);
    stat_sample_free(timestat);

    return MPIPERF_SUCCESS;
}

/*
 * nbcbench_measure_blocking_time: Measures execution time of NBC operation
 *                                 in blocking mode.
 */
int nbcbench_measure_blocking_time(nbcbench_t *bench, nbctest_params_t *params,
                                   double *globaltime, double *localtime)
{
    int i, stage, stage_nruns, nruns, ncorrectruns, nerrors;
    double *stage_exectime = NULL, *exectime_reduced = NULL;
    nbctest_result_t *stage_results = NULL;
    int *stagerc = NULL, *stagerc_reduced = NULL;
    double stagetime, stagetime_max, slotlen;
    stat_sample_t *timestat, *procstat;

    logger_log("Started measure of NBC time in blocking mode");
    timestat = stat_sample_create();
    procstat = stat_sample_create();
    if (timestat == NULL || procstat == NULL) {
        exit_error("No enough memory");
    }

    stage_results = xrealloc(stage_results, sizeof(*stage_results) *
                             TEST_STAGE_NRUNS);
    stage_exectime = xrealloc(stage_exectime, sizeof(*stage_exectime) *
                              TEST_STAGE_NRUNS);
    stagerc = xrealloc(stagerc, sizeof(*stagerc) * TEST_STAGE_NRUNS);
    stagerc_reduced = xrealloc(stagerc_reduced, sizeof(*stagerc_reduced) *
                               TEST_STAGE_NRUNS);
    exectime_reduced = xrealloc(exectime_reduced, sizeof(*exectime_reduced) *
                                TEST_STAGE_NRUNS);

    if (bench->init)
        bench->init(params);

    nruns = 0;
    ncorrectruns = 0;
    stage_nruns = TEST_STAGE_NRUNS_INIT;

    slotlen = 0.0;
    timeslot_initialize_test(params->comm);

    for (stage = -1; ; stage++) {

        logger_flush();
        timeslot_set_length(slotlen);
        timeslot_set_starttime(params->comm);

        /* Run measurements */
        mpiperf_is_measure_started = 1;
        stagetime = hpctimer_wtime();

        for (i = 0; i < stage_nruns; i++) {
            stagerc[i] = bench->blockingop(params, &stage_results[i]);
        }

        stagetime = hpctimer_wtime() - stagetime;
        mpiperf_is_measure_started = 0;

        MPI_Allreduce(&stagetime, &stagetime_max, 1, MPI_DOUBLE, MPI_MAX,
                      params->comm);

        if (stage == -1) {
            /* It was a warmup stage */
            slotlen = stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE;
            stage_nruns = TEST_STAGE_NRUNS;
            logger_log("Warmup stage is finished: %d runs, slotlen = %.6f",
                       TEST_STAGE_NRUNS_INIT, slotlen);
            continue;
        }
        nruns += stage_nruns;

        /* Gather results (time and err. codes) of previous stage */
        for (i = 0; i < stage_nruns; i++) {
            stage_exectime[i] = stage_results[i].totaltime;
        }
        MPI_Allreduce(stagerc, stagerc_reduced, stage_nruns,
                      MPI_INT, MPI_MAX, params->comm);
        MPI_Allreduce(stage_exectime, exectime_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, params->comm);

        nerrors = 0;
        for (i = 0; i < stage_nruns; i++) {
            if (stagerc_reduced[i] == MEASURE_SUCCESS) {
                ncorrectruns++;
                stat_sample_add(timestat, exectime_reduced[i]);
                stat_sample_add(procstat, stage_results[i].totaltime);
                logger_log("NBC time in blocking mode (stage %d, run %d): %.6f",
                           stage, i, exectime_reduced[i]);
            } else {
                /*
                 * Some of processes at the measure #i
                 * was started after established time or terminated
                 * after time slot deadline or collop was failed.
                 */
                nerrors++;
            }
        }

        logger_log("Stage %d is finished: %d runs, %d invalid runs, RSE = %.2f",
                   stage, stage_nruns, nerrors, stat_sample_stderr_rel(timestat));

        /* Check exit condition */
        if (ncorrectruns >= BLOCKINGTIME_NCORRECTRUNS_MIN &&
            (stat_sample_stderr_rel(timestat) <= BLOCKINGTIME_RSE_MAX ||
             ncorrectruns >= BLOCKINGTIME_NCORRECTRUNS_MAX))
        {
            break;
        }

        /* Adjust timeslot parameters */
        if (nerrors > stage_nruns / 4.0) {
            slotlen = stat_fmax2(2.0 * slotlen,
                                 stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE);
            logger_log("Corrected timeslot length: %.6f", slotlen);
        }

    } /* stages */

    logger_log("Test is finished: %d runs, %d correct runs, RSE = %.2f",
               nruns, ncorrectruns, stat_sample_stderr_rel(timestat));

    if (bench->free)
        bench->free();

    free(exectime_reduced);
    free(stagerc_reduced);
    free(stagerc);
    free(stage_exectime);
    free(stage_results);

    /* *globaltime = stat_sample_mean(timestat); */
    *localtime = stat_sample_mean(procstat);
    MPI_Allreduce(localtime, globaltime, 1, MPI_DOUBLE, MPI_MAX, params->comm);

    stat_sample_free(timestat);
    stat_sample_free(procstat);

    return MPIPERF_SUCCESS;
}

int nbcbench_simulate_computing(nbctest_params_t *params,
                                MPI_Request *request, nbctest_result_t *result)
{
    double test_callstep, test_calltime;
    double t, tt, endtime;
    int flag;

    result->ntests = 0;
    if (params->comptime <= 0.0) {
        result->realcomptime = 0.0;
        return MPIPERF_SUCCESS;
    }

    /* TODO: How much test we need to call? Is it depend on message size, nprocs? */
    test_callstep = params->comptime / TESTS_PER_COMPTIME;

    t = hpctimer_wtime();
    endtime = t + params->comptime;
    test_calltime = t + test_callstep;

    while ((tt = hpctimer_wtime()) < endtime) {
        if (tt > test_calltime) {
            MPI_Test(request, &flag, MPI_STATUS_IGNORE);
            result->ntests++;
            test_calltime += test_callstep;
        }
    }
    result->realcomptime = hpctimer_wtime() - t;
    return MPIPERF_SUCCESS;
}

void print_nbcbench_info()
{
    int i;

    printf("=== Non-blocking collective operations ===\n");
    for (i = 0; i < NELEMS(nbcbenchtab); i++) {
        if (nbcbenchtab[i].printinfo) {
            nbcbenchtab[i].printinfo(&nbcbenchtab[i]);
        }
    }
}

nbcbench_t *lookup_nbcbench(const char *name)
{
    int i;

    for (i = 0; i < NELEMS(nbcbenchtab); i++) {
        if (strcasecmp(nbcbenchtab[i].name, name) == 0) {
            return &nbcbenchtab[i];
        }
    }
    return NULL;
}

int report_write_nbcbench_header(nbcbench_t *bench)
{
    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;

    printf("# Characteristics of measurements:\n");

    if (mpiperf_nbcbench_mode == NBCBENCH_OVERLAP) {
        printf("#   Procs: total number of processes\n");
        printf("#   Count: count of elements in send/recv buffer\n");
        printf("#   BlockingTime: time of NBC in blocking mode (Iop + Wait)\n");
        printf("#   CompTime: time of computations\n");
        printf("#   TRuns: total number of measurements (valid and invalid runs)\n");
        printf("#   CRuns: number of correct measurements (only valid runs)\n");
        printf("#   RSE: relative standard error of total time (StdErr / Mean) - max of all procs.\n");
        printf("#   Init: time of NBC issue\n");
        printf("#   Wait: time of MPI_Wait\n");
        printf("#   CompTimeReal: real time of waiting loop (for CompTime)\n");
        printf("#   Total: Total time of NBC operation with computations (Iop + Comp + Wait)\n");
        printf("#   Overlap: 1 - (Total - CompTime) / BlockingTime\n");
        printf("#\n");
        printf("# Value of BlockingTime, Init, Wait, CompTimeReal, Total, Overlap\n");
        printf("# are computed as max_of_all_procs(mean_of_runs(t[i][1], ..., t[i][CRuns])),\n");
        printf("# where t[i][j] is a time (or other value) of process i at measure j = 1, 2, ..., CRuns\n");
        printf("#\n");
        printf("# -------------------------------------------------------------------------------------\n");
        printf("# Benchmark: %s\n", bench->name);
        printf("# Benchmarking mode: overlap measure (Iop + Comp + Wait)\n");
        if (mpiperf_timescale == TIMESCALE_SEC) {
            printf("# [Procs] [Count]     [BlockingTime] [CompTime]   [TRuns] [CRuns] [RSE]      [Init]       [Wait]       [CompTimeReal] [Total]      [Overlap]\n");
        } else {
            /* usec */
            printf("# [Procs] [Count]     [BlockingTime] [CompTime]     [TRuns] [CRuns] [RSE]      [Init]         [Wait]         [CompTimeReal] [Total]        [Overlap]\n");
        }
    } else {
        /*
         * Blocking mode
         */
        printf("#   Procs: total number of processes\n");
        printf("#   Count: count of elements in send/recv buffer\n");
        printf("#   TRuns: total number of measurements (valid and invalid runs)\n");
        printf("#   CRuns: number of correct measurements (only valid runs)\n");
        printf("#   RSE: relative standard error of total time (StdErr / Mean)\n");
        printf("#   Init: time of NBC issue\n");
        printf("#   Wait: time of MPI_Wait\n");
        printf("#   Total: Total time of NBC operation (Iop + Wait)\n");
        printf("#\n");
        printf("# Value of Init, Wait, Total are computed as \n");
        printf("# mean_of_runs(max_of_all_procs(t[0][j], ..., t[Procs - 1][j])),\n");
        printf("# where t[i][j] is a time of process i at measure j = 1, 2, ..., CRuns\n");
        printf("#\n");
        printf("# -------------------------------------------------------------------------------------\n");
        printf("# Benchmark: %s\n", bench->name);
        printf("# Benchmarking mode: blocking time (Iop + Wait)\n");
        if (mpiperf_timescale == TIMESCALE_SEC) {
            printf("# [Procs] [Count]     [TRuns] [CRuns] [RSE]      [Init]       [Wait]       [Total]\n");
        } else {
            /* usec */
            printf("# [Procs] [Count]     [TRuns] [CRuns] [RSE]      [Init]         [Wait]         [Total]\n");
        }
    }
    printf("# -------------------------------------------------------------------------------------\n");
    return MPIPERF_SUCCESS;
}

int report_write_nbcbench_procstat_header(nbcbench_t *bench)
{
    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;

    fprintf(mpiperf_repstream, "# Characteristics of measurements:\n");

    if (mpiperf_nbcbench_mode == NBCBENCH_OVERLAP) {
        fprintf(mpiperf_repstream, "#   Procs: total number of processes\n");
        fprintf(mpiperf_repstream, "#   Count: count of elements in send/recv buffer\n");
        fprintf(mpiperf_repstream, "#   Rank: rank of process\n");
        fprintf(mpiperf_repstream, "#   BlockingTime: time of NBC in blocking mode (Iop + Wait)\n");
        fprintf(mpiperf_repstream, "#   CompTime: time of computations\n");
        fprintf(mpiperf_repstream, "#   TRuns: total number of measurements (valid and invalid runs)\n");
        fprintf(mpiperf_repstream, "#   CRuns: number of correct measurements (only valid runs)\n");
        fprintf(mpiperf_repstream, "#   RSE: relative standard error of total time (StdErr / Mean)\n");
        fprintf(mpiperf_repstream, "#   Init: time of NBC issue\n");
        fprintf(mpiperf_repstream, "#   Wait: time of MPI_Wait\n");
        fprintf(mpiperf_repstream, "#   CompTimeReal: real time of waiting loop (for CompTime)\n");
        fprintf(mpiperf_repstream, "#   Total: total time of NBC operation with computations (Iop + Comp + Wait)\n");
        fprintf(mpiperf_repstream, "#   Overlap: 1 - (Total - CompTime) / BlockingTime\n");
        fprintf(mpiperf_repstream, "#\n");
        fprintf(mpiperf_repstream, "# Value of BlockingTime, Init, Wait, CompTimeReal, Total, Overlap in process i\n");
        fprintf(mpiperf_repstream, "# are computed as mean_of_runs(t[i][1], ..., t[i][CRuns])),\n");
        fprintf(mpiperf_repstream, "# where t[i][j] is a time (or other value) of process i at measure j = 1, 2, ..., CRuns\n");
        fprintf(mpiperf_repstream, "#\n");
        fprintf(mpiperf_repstream, "# ------------------------------------------------------------------\n");
        fprintf(mpiperf_repstream, "# Benchmark: %s\n", bench->name);
        fprintf(mpiperf_repstream, "# Benchmarking mode: overlap measure (Iop + Comp + Wait)\n");
        if (mpiperf_timescale == TIMESCALE_SEC) {
            fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [BlockingTime] [CompTime]   [TRuns] [CRuns] [RSE]      [Init]       [Wait]       [CompTimeReal] [Total]      [Overlap]\n");
        } else {
            /* usec */
            fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [BlockingTime] [CompTime]     [TRuns] [CRuns] [RSE]      [Init]         [Wait]         [CompTimeReal] [Total]        [Overlap]\n");
        }
        fprintf(mpiperf_repstream, "# ------------------------------------------------------------------\n");
    } else {
        /*
         * Blocking mode
         */
        fprintf(mpiperf_repstream, "#   Procs: total number of processes\n");
        fprintf(mpiperf_repstream, "#   Count: count of elements in send/recv buffer\n");
        fprintf(mpiperf_repstream, "#   Rank: rank of process\n");
        fprintf(mpiperf_repstream, "#   TRuns: total number of measurements (valid and invalid runs)\n");
        fprintf(mpiperf_repstream, "#   CRuns: number of correct measurements (only valid runs)\n");
        fprintf(mpiperf_repstream, "#   RSE: relative standard error of total time (StdErr / Mean)\n");
        fprintf(mpiperf_repstream, "#   Init: time of NBC issue\n");
        fprintf(mpiperf_repstream, "#   Wait: time of MPI_Wait\n");
        fprintf(mpiperf_repstream, "#   Total: total time of NBC operation (Iop + Wait)\n");
        fprintf(mpiperf_repstream, "#\n");
        fprintf(mpiperf_repstream, "# Value of Init, Wait, Total in process i\n");
        fprintf(mpiperf_repstream, "# are computed as mean_of_runs(t[i][1], ..., t[i][CRuns])),\n");
        fprintf(mpiperf_repstream, "# where t[i][j] is a time of process i at measure j = 1, 2, ..., CRuns\n");
        fprintf(mpiperf_repstream, "#\n");
        fprintf(mpiperf_repstream, "# --------------------------------------------------------------------------------------------\n");
        fprintf(mpiperf_repstream, "# Benchmark: %s\n", bench->name);
        fprintf(mpiperf_repstream, "# Benchmarking mode: blocking time (Iop + Wait) measure\n");
        if (mpiperf_timescale == TIMESCALE_SEC) {
            fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [TRuns] [CRuns] [RSE]      [Init]       [Wait]       [Total]\n");
        } else {
            /* usec */
            fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [TRuns] [CRuns] [RSE]      [Init]         [Wait]         [Total]\n");
        }
        fprintf(mpiperf_repstream, "# --------------------------------------------------------------------------------------------\n");
    }
    fprintf(mpiperf_repstream, "#\n");
    return MPIPERF_SUCCESS;
}

int report_write_nbcbench_overlap(nbcbench_t *bench, nbctest_params_t *params,
                                  int nruns, int ncorrectruns,
                                  double blockingtime,
                                  stat_sample_t *inittimestat,
                                  stat_sample_t *waittimestat,
                                  stat_sample_t *comptimestat,
                                  stat_sample_t *totaltimestat,
                                  stat_sample_t *overlapstat)
{
    enum {
        INITTIME = 0,
        WAITTIME = 1,
        COMPTIME = 2,
        TOTALTIME = 3,
        OVERLAP = 4,
        TOTALRSE = 5
    };
    enum { NSTAT = 6 };
    double stat[NSTAT], allstat[NSTAT];
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-12.6f   %-12.6f %-7d %-7d %-10.2f %-12.6f %-12.6f %-12.6f   %-12.6f %-8.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-14.2f %-14.2f %-7d %-7d %-10.2f %-14.2f %-14.2f %-14.2f %-14.2f %-8.2f\n";
        timescale = 1E6;
    }

    stat[INITTIME] = stat_sample_mean(inittimestat) * timescale;
    stat[WAITTIME] = stat_sample_mean(waittimestat) * timescale;
    stat[COMPTIME] = stat_sample_mean(comptimestat) * timescale;
    stat[TOTALTIME] = stat_sample_mean(totaltimestat) * timescale;
    stat[OVERLAP] = stat_sample_mean(overlapstat);
    stat[TOTALRSE] = stat_sample_stderr_rel(totaltimestat);

    MPI_Reduce(stat, allstat, NSTAT, MPI_DOUBLE, MPI_MAX, mpiperf_master_rank,
               params->comm);

    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;

    printf(fmt, params->nprocs, params->count, blockingtime * timescale,
           params->comptime * timescale, nruns, ncorrectruns,
           allstat[TOTALRSE], allstat[INITTIME], allstat[WAITTIME],
           allstat[COMPTIME], allstat[TOTALTIME], allstat[OVERLAP]);

    return MPIPERF_SUCCESS;
}

int report_write_nbcbench_procstat_overlap(nbcbench_t *bench,
                                           nbctest_params_t *params,
                                           int nruns, int ncorrectruns,
                                           double blockingtime,
                                           stat_sample_t *inittimestat,
                                           stat_sample_t *waittimestat,
                                           stat_sample_t *comptimestat,
                                           stat_sample_t *totaltimestat,
                                           stat_sample_t *overlapstat)
{
    enum {
        INITTIME = 0,
        WAITTIME = 1,
        COMPTIME = 2,
        TOTALTIME = 3,
        OVERLAP = 4,
        BLOCKINGTIME = 5,
        TOTALRSE = 6
    };
    enum { NSTAT = 8 };
    double stat[NSTAT], *allstat = NULL;
    const char *fmt = NULL;
    double timescale = 0;
    int i;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-12.6f   %-12.6f %-7d %-7d %-10.2f %-12.6f %-12.6f %-12.6f   %-12.6f %-8.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-14.2f %-14.2f %-7d %-7d %-10.2f %-14.2f %-14.2f %-14.2f %-14.2f %-8.2f\n";
        timescale = 1E6;
    }

    if (IS_MASTER_RANK)
        allstat = xmalloc(sizeof(*allstat) * NSTAT * params->nprocs);

    stat[INITTIME] = stat_sample_mean(inittimestat) * timescale;
    stat[WAITTIME] = stat_sample_mean(waittimestat) * timescale;
    stat[COMPTIME] = stat_sample_mean(comptimestat) * timescale;
    stat[TOTALTIME] = stat_sample_mean(totaltimestat) * timescale;
    stat[OVERLAP] = stat_sample_mean(overlapstat);
    stat[BLOCKINGTIME] = blockingtime * timescale;
    stat[TOTALRSE] = stat_sample_stderr_rel(totaltimestat);

    MPI_Gather(stat, NSTAT, MPI_DOUBLE, allstat, NSTAT, MPI_DOUBLE,
               mpiperf_master_rank, params->comm);

    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;

    for (i = 0; i < params->nprocs; i++) {
        fprintf(mpiperf_repstream, fmt, params->nprocs, params->count, i,
                allstat[i * NSTAT + BLOCKINGTIME], params->comptime * timescale,
                nruns, ncorrectruns, allstat[i * NSTAT + TOTALRSE],
                allstat[i * NSTAT + INITTIME],
                allstat[i * NSTAT + WAITTIME],
                allstat[i * NSTAT + COMPTIME],
                allstat[i * NSTAT + TOTALTIME],
                allstat[i * NSTAT + OVERLAP]
        );
    }
    free(allstat);
    return MPIPERF_SUCCESS;
}

/*
 * run_nbcbench_blocking: Measures execution time of the NBC operation
 *                        in blocking mode.
 */
int run_nbcbench_blocking(nbcbench_t *bench, nbctest_params_t *params)
{
    int nruns, ncorrectruns;
    stat_sample_t *inittimestat, *waittimestat, *totaltimestat;
    stat_sample_t *inittimestat_local, *waittimestat_local, *totaltimestat_local;

    MPI_Barrier(MPI_COMM_WORLD);
    logger_log("Test (nprocs = %d, count = %d) is started",
               params->nprocs, params->count);

    if (params->comm != MPI_COMM_NULL) {
        /* This process participates in measures */

        inittimestat = stat_sample_create();
        inittimestat_local = stat_sample_create();
        waittimestat = stat_sample_create();
        waittimestat_local = stat_sample_create();
        totaltimestat = stat_sample_create();
        totaltimestat_local = stat_sample_create();

        if (inittimestat == NULL || waittimestat == NULL ||
            totaltimestat == NULL || inittimestat_local == NULL ||
            waittimestat_local == NULL || totaltimestat_local == NULL)
        {
            exit_error("Can't allocate memory for statistic");
        }

        run_nbcbench_blocking_test(bench, params, &nruns, &ncorrectruns,
                                   inittimestat, waittimestat, totaltimestat,
                                   inittimestat_local, waittimestat_local,
                                   totaltimestat_local);

        report_write_nbcbench_blocking(bench, params, nruns, ncorrectruns,
                                       inittimestat, waittimestat, totaltimestat);

        if (mpiperf_perprocreport) {
            report_write_nbcbench_procstat_blocking(bench, params,
                                                    nruns, ncorrectruns,
                                                    inittimestat_local,
                                                    waittimestat_local,
                                                    totaltimestat_local);
        }
        stat_sample_free(inittimestat);
        stat_sample_free(waittimestat);
        stat_sample_free(totaltimestat);
        stat_sample_free(inittimestat_local);
        stat_sample_free(waittimestat_local);
        stat_sample_free(totaltimestat_local);
    }
    return MPIPERF_SUCCESS;
}

/*
 * run_nbcbench_blocking_test: Measures execution time of the NBC operation
 *                             in current process.
 *
 * We use window-based approach for benchmarking NBC.
 */
int run_nbcbench_blocking_test(nbcbench_t *bench,
                               nbctest_params_t *params,
                               int *nruns, int *ncorrectruns,
                               stat_sample_t *inittimestat,
                               stat_sample_t *waittimestat,
                               stat_sample_t *totaltimestat,
                               stat_sample_t *inittimestat_local,
                               stat_sample_t *waittimestat_local,
                               stat_sample_t *totaltimestat_local)
{
    int i, stage, stage_nruns, nerrors;
    int *stagerc = NULL, *stagerc_reduced = NULL;
    nbctest_result_t *stage_results = NULL;
    double stagetime, stagetime_max, slotlen;
    double *stage_total = NULL, *total_reduced = NULL;
    double *stage_init = NULL, *init_reduced = NULL;
    double *stage_wait = NULL, *wait_reduced = NULL;

    stage_total = xrealloc(stage_total, sizeof(double) * TEST_STAGE_NRUNS);
    total_reduced = xrealloc(total_reduced, sizeof(double) * TEST_STAGE_NRUNS);

    stage_init = xrealloc(stage_init, sizeof(double) * TEST_STAGE_NRUNS);
    init_reduced = xrealloc(init_reduced, sizeof(double) * TEST_STAGE_NRUNS);

    stage_wait = xrealloc(stage_wait, sizeof(double) * TEST_STAGE_NRUNS);
    wait_reduced = xrealloc(wait_reduced, sizeof(double) * TEST_STAGE_NRUNS);

    stage_results = xrealloc(stage_results, sizeof(*stage_results) *
                             TEST_STAGE_NRUNS);
    stagerc = xrealloc(stagerc, sizeof(*stagerc) * TEST_STAGE_NRUNS);
    stagerc_reduced = xrealloc(stagerc_reduced, sizeof(*stagerc_reduced) *
                               TEST_STAGE_NRUNS);

    if (bench->init)
        bench->init(params);

    *nruns = 0;
    *ncorrectruns = 0;
    stage_nruns = TEST_STAGE_NRUNS_INIT;

    slotlen = 0.0;
    timeslot_initialize_test(params->comm);

    for (stage = -1; ; stage++) {

        logger_flush();
        timeslot_set_length(slotlen);
        timeslot_set_starttime(params->comm);

        /* Run measurements */
        mpiperf_is_measure_started = 1;
        stagetime = hpctimer_wtime();

        for (i = 0; i < stage_nruns; i++) {
            stagerc[i] = bench->blockingop(params, &stage_results[i]);
        }

        stagetime = hpctimer_wtime() - stagetime;
        mpiperf_is_measure_started = 0;

        MPI_Allreduce(&stagetime, &stagetime_max, 1, MPI_DOUBLE, MPI_MAX,
                      params->comm);

        if (stage == -1) {
            /* It was a warmup stage */
            slotlen = stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE;
            stage_nruns = TEST_STAGE_NRUNS;
            logger_log("Warmup stage is finished: %d runs, slotlen = %.6f",
                       TEST_STAGE_NRUNS_INIT, slotlen);
            continue;
        }
        (*nruns) = (*nruns) + stage_nruns;

        /* Reduce exectime */
        for (i = 0; i < stage_nruns; i++) {
            stage_total[i] = stage_results[i].totaltime;
            stage_init[i] = stage_results[i].inittime;
            stage_wait[i] = stage_results[i].waittime;
        }
        /* Gather data of previous stage */
        MPI_Allreduce(stage_total, total_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, params->comm);
        MPI_Allreduce(stage_init, init_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, params->comm);
        MPI_Allreduce(stage_wait, wait_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, params->comm);
        MPI_Allreduce(stagerc, stagerc_reduced, stage_nruns,
                      MPI_INT, MPI_MAX, params->comm);

        nerrors = 0;
        for (i = 0; i < stage_nruns; i++) {
            if (stagerc_reduced[i] == MEASURE_SUCCESS) {
                (*ncorrectruns)++;
                stat_sample_add(totaltimestat, total_reduced[i]);
                stat_sample_add(totaltimestat_local, stage_total[i]);

                stat_sample_add(inittimestat, init_reduced[i]);
                stat_sample_add(inittimestat_local, stage_init[i]);

                stat_sample_add(waittimestat, wait_reduced[i]);
                stat_sample_add(waittimestat_local, stage_wait[i]);

                logger_log("NBC measure (comptime: %.6f; stage %d; run %d): "
                           "totaltime = %.6f", params->comptime, stage, i,
                           total_reduced[i]);
            } else {
                /*
                 * Some of processes at the measure #i
                 * was started after established time or terminated
                 * after time slot deadline or collop was failed.
                 */
                nerrors++;
            }
        }

        logger_log("Stage %d is finished: %d runs, %d invalid runs, RSE = %.2f",
                   stage, stage_nruns, nerrors, stat_sample_stderr_rel(totaltimestat));

        /* Check exit condition */
        if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
            if ((*ncorrectruns >= mpiperf_nmeasures_max ||
                *nruns >= mpiperf_nruns_max) && (*nruns >= mpiperf_nruns_min))
            {
                break;
            }
        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
            if (((stat_sample_stderr_rel(totaltimestat) <= mpiperf_rse_max) &&
                 *ncorrectruns >= mpiperf_nruns_min) || (*nruns >= mpiperf_nruns_max))
            {
                break;
            }
        }

        /* Adjust timeslot parameters */
        if (nerrors > stage_nruns / 4.0) {
            slotlen = stat_fmax2(2.0 * slotlen,
                                 stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE);
            logger_log("Corrected timeslot length: %.6f", slotlen);
        }

    } /* stages */

    logger_log("Test is finished: %d runs, %d correct runs, RSE = %.2f",
               *nruns, *ncorrectruns, stat_sample_stderr_rel(totaltimestat));

    if (bench->free)
        bench->free();

    free(stage_total);
    free(total_reduced);
    free(stage_init);
    free(init_reduced);
    free(stage_wait);
    free(wait_reduced);
    free(stagerc_reduced);
    free(stagerc);
    free(stage_results);

    return MPIPERF_SUCCESS;
}

int report_write_nbcbench_blocking(nbcbench_t *bench,
                                   nbctest_params_t *params,
                                   int nruns, int ncorrectruns,
                                   stat_sample_t *inittimestat,
                                   stat_sample_t *waittimestat,
                                   stat_sample_t *totaltimestat)
{
    const char *fmt = NULL;
    double timescale = 0;

    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-7d %-10.2f %-12.6f %-12.6f %-12.6f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-7d %-10.2f %-14.2f %-14.2f %-14.2f\n";
        timescale = 1E6;
    }

    printf(fmt, params->nprocs, params->count, nruns, ncorrectruns,
           stat_sample_stderr_rel(totaltimestat),
           stat_sample_mean(inittimestat) * timescale,
           stat_sample_mean(waittimestat) * timescale,
           stat_sample_mean(totaltimestat) * timescale);

    return MPIPERF_SUCCESS;
}

int report_write_nbcbench_procstat_blocking(nbcbench_t *bench,
                                            nbctest_params_t *params,
                                            int nruns, int ncorrectruns,
                                            stat_sample_t *inittimestat,
                                            stat_sample_t *waittimestat,
                                            stat_sample_t *totaltimestat)
{
    enum {
        INITTIME = 0,
        WAITTIME = 1,
        TOTALTIME = 2,
        TOTALRSE = 3
    };
    enum { NSTAT = 4 };
    double stat[NSTAT], *allstat = NULL;
    const char *fmt = NULL;
    double timescale = 0;
    int i;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-7d %-7d %-10.2f %-12.6f %-12.6f %-12.6f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-7d %-7d %-10.2f %-14.2f %-14.2f %-14.2f\n";
        timescale = 1E6;
    }

    if (IS_MASTER_RANK)
        allstat = xmalloc(sizeof(*allstat) * NSTAT * params->nprocs);

    stat[INITTIME] = stat_sample_mean(inittimestat) * timescale;
    stat[WAITTIME] = stat_sample_mean(waittimestat) * timescale;
    stat[TOTALTIME] = stat_sample_mean(totaltimestat) * timescale;
    stat[TOTALRSE] = stat_sample_stderr_rel(totaltimestat);

    MPI_Gather(stat, NSTAT, MPI_DOUBLE, allstat, NSTAT, MPI_DOUBLE,
               mpiperf_master_rank, params->comm);

    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;

    for (i = 0; i < params->nprocs; i++) {
        fprintf(mpiperf_repstream, fmt, params->nprocs, params->count, i,
                nruns, ncorrectruns,
                allstat[i * NSTAT + TOTALRSE],
                allstat[i * NSTAT + INITTIME],
                allstat[i * NSTAT + WAITTIME],
                allstat[i * NSTAT + TOTALTIME]
        );
    }
    free(allstat);
    return MPIPERF_SUCCESS;
}
