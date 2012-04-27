/*
 * bench_pt2pt.c: Functions for benchmarking MPI point-to-point routines.
 *
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <mpi.h>

#include "bench_pt2pt.h"
#include "mpiperf.h"
#include "stat.h"
#include "timeslot.h"
#include "report.h"
#include "logger.h"
#include "util.h"
#include "hpctimer.h"
#include "bench_pt2pt_tab.h"

#define TEST_SLOTLEN_SCALE 1.1

/*
 * run_pt2ptbench: Runs benchmark for point-to-point operation.
 */
int run_pt2ptbench(pt2ptbench_t *bench)
{
    double benchtime;
	pt2pttest_params_t params;

	if (mpiperf_commsize < 2) {
		exit_error("Too few processes in communicator: %d", mpiperf_commsize);
	}

	if (mpiperf_synctype == SYNC_NONE) {
		exit_error("nosync mode is not supported for point-to-point operations");
	}

	benchtime = hpctimer_wtime();
    timeslot_initialize();

	report_write_header();
    report_write_pt2ptbench_header(bench);

    params.nprocs = 2;
	params.comm = createcomm(MPI_COMM_WORLD, params.nprocs);

	/* For each data size (count) */
	for (params.count = mpiperf_count_min;
		 params.count <= mpiperf_count_max; )
	{
		run_pt2ptbench_test(bench, &params);

		if (mpiperf_count_step_type == STEP_TYPE_MUL) {
			params.count *= mpiperf_count_step;
		} else {
			params.count += mpiperf_count_step;
		}
	}

	if (params.comm != MPI_COMM_NULL)
		MPI_Comm_free(&params.comm);

	timeslot_finalize();
    MPI_Barrier(MPI_COMM_WORLD);
    benchtime = hpctimer_wtime() - benchtime;
    if (IS_MASTER_RANK) {
        report_printf("# Elapsed time: %.6f sec.\n", benchtime);
    }
	return MPIPERF_SUCCESS;
}

/*
 * run_pt2ptbench_test: Measures execution time of the point-to-point operation
 *                      for given parameters (data size, nprocs).
 */
int run_pt2ptbench_test(pt2ptbench_t *bench, pt2pttest_params_t *params)
{
    int nruns, ncorrectruns;
    double *exectime = NULL;

    MPI_Barrier(MPI_COMM_WORLD);
    logger_log("Test (nprocs = %d, count = %d) is started",
    		   params->nprocs, params->count);

    if (params->comm != MPI_COMM_NULL) {
		/* This process participates in measures */
        run_pt2ptbench_test_synctime(bench, params, &exectime, &nruns,
        		                     &ncorrectruns);
        report_write_pt2pttest_synctime(bench, params, exectime, nruns,
        		                        ncorrectruns);
        free(exectime);
	}
    return MPIPERF_SUCCESS;
}

/*
 * run_pt2ptbench_test_synctime: Measures execution time of point-to-point
 *                               operation for given parameters.
 * Exectution time of point-to-point operation is a time of master process.
 */
int run_pt2ptbench_test_synctime(pt2ptbench_t *bench, pt2pttest_params_t *params,
		                         double **exectime, int *nmeasurements,
		                         int *ncorrect_measurements)
{
    int i, stage, stage_nruns, nruns, ncorrectruns, exectime_size, nerrors;
    double *stage_exectime = NULL;
    int *stagerc = NULL;
    double stagetime, stagetime_max, slotlen;
    stat_sample_t *timestat = NULL;
	int breakflag;

    if (IS_MASTER_RANK) {
    	if ( (timestat = stat_sample_create()) == NULL) {
        	exit_error("No enough memory");
    	}

    	/* Result execution time (exectime[i] - time off master process) */
    	exectime_size = TEST_STAGE_NRUNS;
    	*exectime = xrealloc(*exectime, sizeof(**exectime) * exectime_size);
    }

    stage_nruns = TEST_STAGE_NRUNS_INIT;
    stage_exectime = xrealloc(stage_exectime, sizeof(*stage_exectime) *
    		                  TEST_STAGE_NRUNS);
    stagerc = xrealloc(stagerc, sizeof(*stagerc) * TEST_STAGE_NRUNS);

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
            stagerc[i] = bench->op(params, &stage_exectime[i]);
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

        if (IS_MASTER_RANK) {
	        /* Allocate memory for stage results */
	        if (exectime_size < (ncorrectruns + stage_nruns)) {
		        exectime_size *= TEST_REALLOC_GROWSTEP;
        	    *exectime = xrealloc(*exectime, sizeof(**exectime) * exectime_size);
        	}

        	nerrors = 0;
        	for (i = 0; i < stage_nruns; i++) {
            	if (stagerc[i] == MEASURE_SUCCESS) {
            		/* Add result to global list */
                	(*exectime)[ncorrectruns++] = stage_exectime[i];
                	stat_sample_add(timestat, stage_exectime[i]);
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
        	breakflag = 0;

        	if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
            	if ((ncorrectruns >= mpiperf_nmeasures_max ||
                	nruns >= mpiperf_nruns_max) && (nruns >= mpiperf_nruns_min))
            	{
                	breakflag = 1;
            	}
	        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
    	        if (((stat_sample_stderr_rel(timestat) <= mpiperf_rse_max) &&
        	        ncorrectruns >= mpiperf_nruns_min) || (nruns >= mpiperf_nruns_max))
            	{
            		breakflag = 1;
            	}
        	}
        } /* master */

        /* Check exit condition */
        MPI_Bcast(&breakflag, 1, MPI_INT, mpiperf_master_rank, params->comm);
        if (breakflag)
            break;

        /* Adjust timeslot parameters */
        MPI_Bcast(&nerrors, 1, MPI_INT, mpiperf_master_rank, params->comm);
        if (nerrors > stage_nruns / 4.0) {
            slotlen = stat_fmax2(2.0 * slotlen,
                                 stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE);
            logger_log("Corrected timeslot length: %.6f", slotlen);
        }

    } /* stages */

    if (IS_MASTER_RANK) {
    	logger_log("Test is finished: %d runs, %d correct runs, RSE = %.2f",
        	       nruns, ncorrectruns, stat_sample_stderr_rel(timestat));
    }

    *nmeasurements = nruns;
    *ncorrect_measurements = ncorrectruns;

    if (bench->free)
    	bench->free();

    free(stage_exectime);
    free(stagerc);

    if (IS_MASTER_RANK)
    	stat_sample_free(timestat);

    return MPIPERF_SUCCESS;
}

void print_pt2ptbench_info()
{
    int i;

    printf("=== Point-to-point operations ===\n");
    for (i = 0; i < NELEMS(pt2ptbenchtab); i++) {
        if (pt2ptbenchtab[i].printinfo) {
        	pt2ptbenchtab[i].printinfo(&pt2ptbenchtab[i]);
        }
    }
}

pt2ptbench_t *lookup_pt2ptbench(const char *name)
{
    int i;

    for (i = 0; i < NELEMS(pt2ptbenchtab); i++) {
        if (strcasecmp(pt2ptbenchtab[i].name, name) == 0) {
            return &pt2ptbenchtab[i];
        }
    }
    return NULL;
}

int report_write_pt2ptbench_header(pt2ptbench_t *bench)
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
        printf("#\n");
        printf("# ------------------------------------------------------------------\n");
        printf("# %s time (time of the master process)\n", bench->name);
        printf("# Confidence level (CL): %d%%\n", mpiperf_confidence_level);
        printf("# ------------------------------------------------------------------\n");
        if (mpiperf_timescale == TIMESCALE_SEC) {
        	printf("# [Procs] [Count]     [TRuns] [CRuns] [FRuns] [Mean]       [RSE]      [StdErr]     [Min]        [Max]        [Err]        [CI LB]      [CI UB]      [RelErr]\n");
        } else {
            /* usec */
            printf("# [Procs] [Count]     [TRuns] [CRuns] [FRuns] [Mean]         [RSE]      [StdErr]       [Min]          [Max]          [Err]          [CI LB]        [CI UB]        [RelErr]\n");
        }
        printf("#\n");
    }
    return MPIPERF_SUCCESS;
}

int report_write_pt2pttest_synctime(pt2ptbench_t *bench,
  		                            pt2pttest_params_t *params,
		                            double *exectime, int nruns,
		                            int ncorrectruns)
{
    double exectime_mean, exectime_stderr, exectime_rse, exectime_min,
           exectime_max, exectime_errrel;
    double exectime_ci_lb = 0.0, exectime_ci_ub = 0.0, exectime_err = 0.0;
    int nresultruns = 0;
    stat_sample_t *sample;
    const char *fmt = NULL;
    double timescale;

    if (!IS_MASTER_RANK) {
        return MPIPERF_SUCCESS;
    }

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

    if (ncorrectruns == 0) {
        printf(fmt, params->nprocs, params->count, nruns, 0, 0, 0.0, 0.0,
               0.0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
        return MPIPERF_SUCCESS;
    }

    /* Remove outliers */
    nresultruns = ncorrectruns;
    if (mpiperf_statanalysis) {
    	/* Remove 25% of minimal values and 25% of maximal values */
        nresultruns = stat_dataset_remove_outliers(exectime,
                                                   ncorrectruns,
                                                   OUTLIERS_FILTER_MIN,
                                                   OUTLIERS_FILTER_MAX);
	}

    if ( (sample = stat_sample_create()) == NULL)
	    return MPIPERF_FAILURE;

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
    return MPIPERF_SUCCESS;
}
