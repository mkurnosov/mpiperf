/*
 * bench_coll.c: Functions for benchmarking MPI collective routines.
 *
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <mpi.h>

#include "bench_coll.h"
#include "mpiperf.h"
#include "timeslot.h"
#include "report.h"
#include "logger.h"
#include "stat.h"
#include "util.h"
#include "hpctimer.h"
#include "bench_coll_tab.h"

#define TEST_SLOTLEN_SCALE 1.1

/* run_collbench: Runs benchmark for collective operation */
int run_collbench(collbench_t *bench)
{
    double benchtime;
	colltest_params_t params;

	benchtime = hpctimer_wtime();
    timeslot_initialize();

	report_write_header();
    report_write_collbench_header(bench);
    if (mpiperf_perprocreport)
    	report_write_collbench_procstat_header(bench);

	/* For each communicator size */
	for (params.nprocs = mpiperf_nprocs_min;
		 params.nprocs <= mpiperf_nprocs_max; )
	{
		params.comm = createcomm(MPI_COMM_WORLD, params.nprocs);

		/* For each data size (count) */
		for (params.count = mpiperf_count_min;
			 params.count <= mpiperf_count_max; )
		{
			/* Test collective operation for given nprocs and count */
			run_collbench_test(bench, &params);

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
 * run_collbench_test: Measures execution time of the collective
 *                     for given parameters (data size, nprocs).
 */
int run_collbench_test(collbench_t *bench, colltest_params_t *params)
{
    int nruns, ncorrectruns;
    double time, *exectime = NULL;
    stat_sample_t *procstat = NULL;

    MPI_Barrier(MPI_COMM_WORLD);
    logger_log("Test (nprocs = %d, count = %d) is started",
    		   params->nprocs, params->count);

    if (params->comm != MPI_COMM_NULL) {
		/* This process participates in measures */
    	if (mpiperf_perprocreport)
	       	procstat = stat_sample_create();

        if (mpiperf_synctype == SYNC_TIME) {
        	run_collbench_test_synctime(bench, params, &exectime, &nruns,
        			                    &ncorrectruns, procstat);
        	report_write_colltest_synctime(bench, params, exectime, nruns,
        			                       ncorrectruns);
    		if (mpiperf_perprocreport)
        		report_write_collbench_procstat_synctime(bench, params, procstat);
        } else {
			run_collbench_test_nosync(bench, params, &time, &nruns, procstat);
            report_write_colltest_nosync(bench, params, time, nruns);
    		if (mpiperf_perprocreport)
	    		report_write_collbench_procstat_nosync(bench, params, procstat);
        }

    	if (mpiperf_perprocreport)
        	stat_sample_free(procstat);
        free(exectime);
    }

    return MPIPERF_SUCCESS;
}

/*
 * run_collbench_test_synctime: Measures execution time of collective operation
 *                              for given parameters (nprocs, count, etc.).
 *
 * Exectution time of collective operation is maximum time off all processes.
 * We use modification of time window-based approach for benchmarking collectives [*].
 *
 * [*] Thomas Worsch, Ralf Reussner, Werner Augustin. On Benchmarking Collective
 *     MPI Operations // In Proc. of PVM/MPI, 2002, pp. 271-279.
 */
int run_collbench_test_synctime(collbench_t *bench, colltest_params_t *params,
		                        double **exectime, int *nmeasurements,
		                        int *ncorrect_measurements, stat_sample_t *procstat)
{
    int i, stage, stage_nruns, nruns, ncorrectruns, exectime_size, nerrors;
    double *stage_exectime = NULL, *exectime_reduced = NULL;
    int *stagerc = NULL, *stagerc_reduced = NULL;
    double stagetime, stagetime_max, slotlen;
    stat_sample_t *timestat;

    if ( (timestat = stat_sample_create()) == NULL) {
        exit_error("No enough memory");
    }

    /* Result execution time (exectime[i] - maximum time off all process) */
    exectime_size = TEST_STAGE_NRUNS;
    *exectime = xrealloc(*exectime, sizeof(**exectime) * exectime_size);

    stage_nruns = TEST_STAGE_NRUNS_INIT;
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
            stagerc[i] = bench->collop(params, &stage_exectime[i]);
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
        MPI_Allreduce(stagerc, stagerc_reduced, stage_nruns,
                      MPI_INT, MPI_MAX, params->comm);
        MPI_Allreduce(stage_exectime, exectime_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, params->comm);

        /* Allocate memory for stage results */
        if (exectime_size < (ncorrectruns + stage_nruns)) {
	        exectime_size *= TEST_REALLOC_GROWSTEP;
            *exectime = xrealloc(*exectime, sizeof(**exectime) * exectime_size);
        }

        nerrors = 0;
        for (i = 0; i < stage_nruns; i++) {
            if (stagerc_reduced[i] == MEASURE_SUCCESS) {
            	/* Add result to global list */
                (*exectime)[ncorrectruns++] = exectime_reduced[i];
                stat_sample_add(timestat, exectime_reduced[i]);

                if (procstat) {
                	/* Add to perprocess report */
                	stat_sample_add(procstat, stage_exectime[i]);
                }
                logger_log("Measured time (stage %d, run %d): %.6f",
                           stage, i, stage_exectime[i]);
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
            if ((ncorrectruns >= mpiperf_nmeasures_max ||
                nruns >= mpiperf_nruns_max) && (nruns >= mpiperf_nruns_min))
            {
                break;
            }
        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
            if (((stat_sample_stderr_rel(timestat) <= mpiperf_rse_max) &&
                 ncorrectruns >= mpiperf_nruns_min) || (nruns >= mpiperf_nruns_max))
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
               nruns, ncorrectruns, stat_sample_stderr_rel(timestat));

    *nmeasurements = nruns;
    *ncorrect_measurements = ncorrectruns;

    if (bench->free)
    	bench->free();

    free(stage_exectime);
    free(stagerc);
    free(stagerc_reduced);
    free(exectime_reduced);
    stat_sample_free(timestat);

    return MPIPERF_SUCCESS;
}

/*
 * run_collbench_test_nosync: Measures execution time of collective operation
 *                            for given parameters.
 *
 * This test implements pipelined measurements (without synchronization of runs).
 */
int run_collbench_test_nosync(collbench_t *bench, colltest_params_t *params,
		                      double *exectime, int *nmeasurements,
		                      stat_sample_t *procstat)
{
	double t, time;
	int i;

	if (bench->init)
    	bench->init(params);

    /* Warmup */
	bench->collop(params, &time);
    MPI_Barrier(params->comm);

	/* Run measurements */
    mpiperf_is_measure_started = 1;
    t = hpctimer_wtime();
    for (i = 0; i < mpiperf_nruns_max; i++) {
    	bench->collop(params, &time);
    }
    t = (hpctimer_wtime() - t) / mpiperf_nruns_max;
    mpiperf_is_measure_started = 0;

    MPI_Allreduce(&t, exectime, 1, MPI_DOUBLE, MPI_MAX, params->comm);

    if (procstat)
		stat_sample_add(procstat, t);

    *nmeasurements = mpiperf_nruns_max;

    if (bench->free)
        bench->free();

    return MPIPERF_SUCCESS;
}

/* print_collbench_info: Prints list of collective benchmarks */
void print_collbench_info()
{
    int i;

    printf("=== MPI 2.2 Collective operations ===\n");
    for (i = 0; i < NELEMS(collbenchtab); i++) {
        if (collbenchtab[i].printinfo) {
        	collbenchtab[i].printinfo(&collbenchtab[i]);
        }
    }
}

/* lookup_collbench: Lookups benchmark by name */
collbench_t *lookup_collbench(const char *name)
{
    int i;

    for (i = 0; i < NELEMS(collbenchtab); i++) {
        if (strcasecmp(collbenchtab[i].name, name) == 0) {
            return &collbenchtab[i];
        }
    }
    return NULL;
}

int report_write_collbench_header(collbench_t *bench)
{
    if (IS_MASTER_RANK) {
        printf("# Characteristics of measurements:\n");
        printf("#   Procs - total number of processes\n");
        printf("#   Count - count of elements in send/recv buffer\n");
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

    	if (mpiperf_synctype == SYNC_TIME) {
        	printf("#\n");
        	printf("# Value of Mean are computed as\n");
        	printf("# mean_of_runs(max_of_all_procs(t[0][j], ..., t[Procs - 1][j])),\n");
        	printf("# where t[i][j] is a time of process i at measure j = 1, 2, ..., CRuns\n");
            printf("#\n");
            printf("# ------------------------------------------------------------------\n");
            printf("# Benchmark: %s\n", bench->name);
            printf("# Confidence level (CL): %d%%\n", mpiperf_confidence_level);
            printf("# ------------------------------------------------------------------\n");
            if (mpiperf_timescale == TIMESCALE_SEC) {
            	printf("# [Procs] [Count]     [TRuns] [CRuns] [FRuns] [Mean]       [RSE]      [StdErr]     [Min]        [Max]        [Err]        [CI LB]      [CI UB]      [RelErr]\n");
            } else {
                /* usec */
                printf("# [Procs] [Count]     [TRuns] [CRuns] [FRuns] [Mean]         [RSE]      [StdErr]       [Min]          [Max]          [Err]          [CI LB]        [CI UB]        [RelErr]\n");
            }
            printf("#\n");
    	} else {
			printf("#\n");
    	    printf("# ------------------------------------------------------------------\n");
    	    printf("# Benchmark: %s\n", bench->name);
            printf("# Pipelined measurements\n");
    	    printf("# ------------------------------------------------------------------\n");
    	    if (mpiperf_timescale == TIMESCALE_SEC) {
    	    	printf("# [Procs] [Count]     [TRuns] [Mean]\n");
    	    } else {
    	    	/* usec */
    	        printf("# [Procs] [Count]     [TRuns] [Mean]\n");
    	    }
    	    printf("#\n");
    	}
    }
    return MPIPERF_SUCCESS;
}

int report_write_colltest_synctime(collbench_t *bench, colltest_params_t *params,
		                           double *exectime, int nruns, int ncorrectruns)
{
    double exectime_mean, exectime_stderr, exectime_rse, exectime_min,
           exectime_max, exectime_errrel;
    double exectime_ci_lb = 0.0, exectime_ci_ub = 0.0, exectime_err = 0.0;
    int nresultruns = 0;
    stat_sample_t *sample;
    const char *fmt = NULL;
    double timescale;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-7d %-7d %-12.6f %-10.2f %-12.6f %-12.6f "
        	  "%-12.6f %-12.6f %-12.6f %-12.6f %-10.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-7d %-7d %-14.2f %-10.2f %-14.2f %-14.2f "
        	"%-14.2f %-14.2f %-14.2f %-14.2f %-10.2f\n";
        timescale = 1E6;
    }

    if (ncorrectruns == 0 && IS_MASTER_RANK) {
        printf(fmt, params->nprocs, params->count, nruns, 0, 0, 0.0, 0.0,
               0.0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
        return MPIPERF_SUCCESS;
    }

    if (IS_MASTER_RANK) {
        if ( (sample = stat_sample_create()) == NULL)
            return MPIPERF_FAILURE;

        /* Remove outliers */
        nresultruns = ncorrectruns;
        if (mpiperf_statanalysis) {
            /* Remove 25% of minimal values and 25% of maximal values */
            nresultruns = stat_dataset_remove_outliers(exectime,
                                                       ncorrectruns,
                                                       OUTLIERS_FILTER_MIN,
                                                       OUTLIERS_FILTER_MAX);
        }

        stat_sample_add_dataset(sample, exectime, nresultruns);
        exectime_mean = stat_sample_mean(sample) * timescale;
        exectime_stderr = stat_sample_stderr(sample) * timescale;
        exectime_rse = stat_sample_stderr_rel(sample);
        exectime_min = stat_sample_min(sample) * timescale;
        exectime_max = stat_sample_max(sample) * timescale;

        /* Build confidence interval */
        stat_sample_confidence_interval(sample, mpiperf_confidence_level_type,
                                        &exectime_ci_lb, &exectime_ci_ub,
                                        &exectime_err);
        exectime_ci_lb *= timescale;
        exectime_ci_ub *= timescale;
        exectime_err *= timescale;
        exectime_errrel = (exectime_err > 0.0) ? exectime_err / exectime_mean : 0.0;

        printf(fmt, params->nprocs, params->count, nruns, ncorrectruns,
               nresultruns, exectime_mean, exectime_rse, exectime_stderr,
               exectime_min, exectime_max, exectime_err, exectime_ci_lb,
               exectime_ci_ub, exectime_errrel);

        stat_sample_free(sample);
    }
    return MPIPERF_SUCCESS;
}

int report_write_colltest_nosync(collbench_t *bench, colltest_params_t *params,
		                         double exectime_local, int nruns)
{
    double exectime;
    const char *fmt = NULL;
    double timescale = 0;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-12.6f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-14.2f\n";
        timescale = 1E6;
    }

    if (nruns == 0 && IS_MASTER_RANK) {
        printf(fmt, params->nprocs, params->count, nruns, 0.0);
        return MPIPERF_SUCCESS;
    }

    /* Gather maximum time on all processes */
    MPI_Reduce(&exectime_local, &exectime, 1, MPI_DOUBLE, MPI_MAX,
               mpiperf_master_rank, params->comm);
	exectime *= timescale;

    if (IS_MASTER_RANK) {
        printf(fmt, params->nprocs, params->count, nruns, exectime);
    }
    return MPIPERF_SUCCESS;
}

int report_write_collbench_procstat_header(collbench_t *bench)
{
    if (!IS_MASTER_RANK)
        return MPIPERF_SUCCESS;
    fprintf(mpiperf_repstream, "# Characteristics of measurements:\n");
    fprintf(mpiperf_repstream, "#   Procs - total number of processes\n");
    fprintf(mpiperf_repstream, "#   Count - count of elements in send/recv buffer\n");
    fprintf(mpiperf_repstream, "#   Rank - rank of process\n");
    fprintf(mpiperf_repstream, "#   CRuns - number of correct measurements (only valid runs)\n");
    fprintf(mpiperf_repstream, "#   Mean - arithmetic mean of execution time (based on FRuns)\n");
    fprintf(mpiperf_repstream, "#   RSE - relative standard error (StdErr / Mean)\n");
    fprintf(mpiperf_repstream, "#   StdErr - standard error of the mean: StdDev / sqrt(FRuns)\n");
    fprintf(mpiperf_repstream, "#   Min - minimal value\n");
    fprintf(mpiperf_repstream, "#   Max - miximal value\n");
    fprintf(mpiperf_repstream, "#   CL - confidence level: 90%%, 95%% or 99%%\n");
    fprintf(mpiperf_repstream, "#   Err - error of measurements: t_student * StdErr\n");
    fprintf(mpiperf_repstream, "#   CI LB - lower bound of confidence interval: Mean - Err\n");
    fprintf(mpiperf_repstream, "#   CI UB - upper bound of confidence interval: Mean + Err\n");
    fprintf(mpiperf_repstream, "#   RelErr - relative error of measurements: Err / Mean\n");
    fprintf(mpiperf_repstream, "#\n");

    if (mpiperf_synctype == SYNC_TIME) {
	    fprintf(mpiperf_repstream, "# Value of Mean in process i is computed as\n");
	    fprintf(mpiperf_repstream, "# mean_of_runs(t[i][1], ..., t[i][CRuns])),\n");
	    fprintf(mpiperf_repstream, "# where t[i][j] is a time of process i at measure j = 1, 2, ..., CRuns\n");
		fprintf(mpiperf_repstream, "#\n");
	    fprintf(mpiperf_repstream, "# ------------------------------------------------------------------\n");
	    fprintf(mpiperf_repstream, "# Benchmark: %s \n", bench->name);
	    fprintf(mpiperf_repstream, "# Confidence level (CL): %d%%\n", mpiperf_confidence_level);
	    fprintf(mpiperf_repstream, "# ------------------------------------------------------------------\n");

		if (mpiperf_timescale == TIMESCALE_SEC) {
		  	fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [CRuns] [Mean]       [RSE]      [StdErr]     [Min]        [Max]       [Err]        [CI LB]      [CI UB]      [RelErr]\n");
    	} else {
    		/* usec */
        	fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [CRuns] [Mean]         [RSE]      [StdErr]       [Min]          [Max]         [Err]          [CI LB]        [CI UB]        [RelErr]\n");
    	}
    } else {
    	/* NOSYNC */
	    fprintf(mpiperf_repstream, "# ------------------------------------------------------------------\n");
	    fprintf(mpiperf_repstream, "# Benchmark: %s \n", bench->name);
	    fprintf(mpiperf_repstream, "# Pipelined measurements\n");
	    fprintf(mpiperf_repstream, "# ------------------------------------------------------------------\n");
	    if (mpiperf_timescale == TIMESCALE_SEC) {
		  	fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [TRuns] [Mean]\n");
    	} else {
    		/* usec */
		  	fprintf(mpiperf_repstream, "# [Procs] [Count]     [Rank]  [TRuns] [Mean]\n");
    	}
    }
    fprintf(mpiperf_repstream, "#\n");
    return MPIPERF_SUCCESS;
}

int report_write_collbench_procstat_synctime(collbench_t *bench,
		                                     colltest_params_t *params,
		                                     stat_sample_t *procstat)
{
    enum { NSTAT_PARAMS = 9 };
    double statparams[NSTAT_PARAMS], *allstat = NULL;
    double lb, ub, err, errrel;
    const char *fmt = NULL;
    double timescale = 0;
    int i;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-7d %-12.6f %-10.2f %-12.6f %-12.6f %-12.6f"
              "%-12.6f %-12.6f %-12.6f %-10.2f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-7d %-14.2f %-10.2f %-14.2f %-14.2f %-14.2f"
              "%-14.2f %-14.2f %-14.2f %-10.2f\n";
        timescale = 1E6;
    }

    if (IS_MASTER_RANK) {
        allstat = xmalloc(sizeof(*allstat) * NSTAT_PARAMS * params->nprocs);
    }

    stat_sample_confidence_interval(procstat, mpiperf_confidence_level_type, &lb, &ub, &err);
    statparams[0] = stat_sample_mean(procstat) * timescale;
    statparams[1] = stat_sample_stderr(procstat) * timescale;
    statparams[2] = stat_sample_stderr_rel(procstat);
    statparams[3] = stat_sample_min(procstat) * timescale;
    statparams[4] = stat_sample_max(procstat) * timescale;
    statparams[5] = (double)stat_sample_size(procstat);
    statparams[6] = lb * timescale;
	statparams[7] = ub * timescale;
    statparams[8] = err * timescale;

    /* Gather statistics: T_mem  = O(n) */
    MPI_Gather(statparams, NSTAT_PARAMS, MPI_DOUBLE, allstat, NSTAT_PARAMS,
    		   MPI_DOUBLE, mpiperf_master_rank, params->comm);

    if (IS_MASTER_RANK) {
	    for (i = 0; i < params->nprocs; i++) {
        	if ((int)allstat[i * NSTAT_PARAMS + 5] > 0) {
            	/* Sample is not empty: samplesize > 0 */
                errrel = 0.0;
                if (allstat[i * NSTAT_PARAMS + 8] > 0.0) {
                	/* err > 0 */
                    errrel = allstat[i * NSTAT_PARAMS + 8] /
                             allstat[i * NSTAT_PARAMS + 0];
                }
                fprintf(mpiperf_repstream, fmt, params->nprocs, params->count, i,
					    (int)allstat[i * NSTAT_PARAMS + 5], /* Runs */
                        allstat[i * NSTAT_PARAMS + 0],      /* Mean */
                        allstat[i * NSTAT_PARAMS + 2],      /* RSE */
                        allstat[i * NSTAT_PARAMS + 1],      /* StdErr */
                        allstat[i * NSTAT_PARAMS + 3],      /* Min */
                        allstat[i * NSTAT_PARAMS + 4],      /* Max */
                        allstat[i * NSTAT_PARAMS + 8],      /* Err */
                        allstat[i * NSTAT_PARAMS + 6],      /* CI LB */
                        allstat[i * NSTAT_PARAMS + 7],      /* CI UB */
                        errrel                              /* Err / Mean */
                       );
			} else {
            	/* Empty sample */
				fprintf(mpiperf_repstream, fmt, params->nprocs, params->count, i,
                        0,    /* Runs */
                        0.0,  /* Mean */
                        0.0,  /* RSE */
                        0.0,  /* StdErr */
                        0.0,  /* Min */
                        0.0,  /* Max */
                        0.0,  /* Err */
                        0.0,  /* CI LB */
                        0.0,  /* CI UB */
                        0.0   /* Err / Mean */
                       );
            }
		} /* for nprocs */

	    /* Other processes did't participate in test */
	    /*
	    for (i = params->nprocs; i < mpiperf_commsize; i++) {
			fprintf(mpiperf_repstream, fmt, params->nprocs, params->count, i,
                    0,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    0.0
                   );
	    }
	    */
		/*
		fprintf(mpiperf_repstream, "\n");
		*/
	}

    if (IS_MASTER_RANK) {
        free(allstat);
    }
    return MPIPERF_SUCCESS;
}

int report_write_collbench_procstat_nosync(collbench_t *bench,
		                                   colltest_params_t *params,
		                                   stat_sample_t *procstat)
{
    double exectime, *allstat = NULL;
    const char *fmt = NULL;
    double timescale = 0;
    int i;

    if (mpiperf_timescale == TIMESCALE_SEC) {
        fmt = "  %-7d %-11d %-7d %-7d %-12.6f\n";
        timescale = 1.0;
    } else {
        /* usec */
        fmt = "  %-7d %-11d %-7d %-7d %-14.2f\n";
        timescale = 1E6;
    }

    if (IS_MASTER_RANK) {
        allstat = xmalloc(sizeof(*allstat) * mpiperf_commsize);
    }

    exectime = stat_sample_mean(procstat) * timescale;
    MPI_Gather(&exectime, 1, MPI_DOUBLE, allstat, 1, MPI_DOUBLE,
      		   mpiperf_master_rank, params->comm);

    if (IS_MASTER_RANK) {
	    for (i = 0; i < params->nprocs; i++) {
		    fprintf(mpiperf_repstream, fmt, params->nprocs, params->count, i,
		    		mpiperf_nruns_max, allstat[i]);
        }
    }

    if (IS_MASTER_RANK) {
        free(allstat);
    }
    return MPIPERF_SUCCESS;
}
