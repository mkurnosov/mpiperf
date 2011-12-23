/*
 * bench.c: Functions for benchmarking MPI routines.
 *
 * Copyright (C) 2010-2011 Mikhail Kurnosov
 */

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <mpi.h>

#include "bench.h"
#include "mpiperf.h"
#include "seq.h"
#include "stats.h"
#include "timeslot.h"
#include "report.h"
#include "logger.h"
#include "util.h"
#include "hpctimer.h"
#include "benchmarks/barrier.h"
#include "benchmarks/bcast.h"
#include "benchmarks/gather.h"
#include "benchmarks/gatherv.h"
#include "benchmarks/scatter.h"
#include "benchmarks/scatterv.h"
#include "benchmarks/allgather.h"
#include "benchmarks/allgatherv.h"
#include "benchmarks/alltoall.h"
#include "benchmarks/alltoallv.h"
#include "benchmarks/alltoallw.h"
#include "benchmarks/reduce.h"
#include "benchmarks/allreduce.h"
#include "benchmarks/reduce_scatter_block.h"
#include "benchmarks/reduce_scatter.h"
#include "benchmarks/scan.h"
#include "benchmarks/exscan.h"
#include "benchmarks/waitpattern.h"
#include "benchmarks/send.h"
#include "benchmarks/sendrecv.h"

#include "benchtab.h"

#define NELEMS(v) (sizeof(v) / sizeof((v)[0]))
#define TEST_SLOTLEN_SCALE 1.1

enum {
    TEST_REALLOC_GROWSTEP = 2,
    TEST_STAGE_NRUNS_INIT = 4,
    TEST_STAGE_NRUNS = 8
};

/* bench_initialize: Initializes benchmark before all tests. */
int bench_initialize(bench_t *bench)
{
    if (bench->init) {
        return bench->init(bench);
    }
    return MPIPERF_SUCCESS;
}

/* bench_finalize: */
int bench_finalize(bench_t *bench)
{
    if (bench) {
        if (bench->free) {
            bench->free(bench);
        }
    }
    return MPIPERF_SUCCESS;
}

/* lookup_bench: Returns pointer to benchmark by name or NULL on error. */
bench_t *lookup_bench(const char *name)
{
    int i;

    for (i = 0; i < NELEMS(benchtab); i++) {
        if (strcasecmp(benchtab[i].name, name) == 0) {
            return &benchtab[i];
        }
    }
    return NULL;
}

/* print_benchmarks_info: Prints information about each benchmark. */
void print_benchmarks_info()
{
    int i;

    for (i = 0; i < NELEMS(benchtab); i++) {
        printf("\n=== Benchmark %d ===\n", i + 1);
        if (benchtab[i].printinfo) {
            benchtab[i].printinfo(&benchtab[i]);
        }
    }
}

/*
 * measure_bcast_double: Measures MPI_Bcast function time
 *                       for sending one element of type double.
 *                       This value is used for determining start time
 *                       of the first timeslot.
 */
double measure_bcast_double(MPI_Comm comm)
{
    double buf, totaltime = 0.0, optime, maxtime = 0.0;
    int i, nreps = 3;

    /* Warmup call */
    MPI_Bcast(&buf, 1, MPI_DOUBLE, mpiperf_master_rank, comm);
    /* Measures */
    for (i = 0; i < nreps; i++) {
        MPI_Barrier(comm);
        optime = hpctimer_wtime();
        MPI_Bcast(&buf, 1, MPI_DOUBLE, mpiperf_master_rank, comm);
        optime = hpctimer_wtime() - optime;
        MPI_Reduce(&optime, &maxtime, 1, MPI_DOUBLE, MPI_MAX,
                   mpiperf_master_rank, comm);
        totaltime = stats_fmax2(totaltime, maxtime);
    }
    return totaltime;
}

/*
 * run_collective_bench: Runs benchmark for collective operation.
 *                       Benchmark is a sequence of collective
 *                       communication runs for each value of variable
 *                       parameter (commsize, send count, etc).
 */
int run_collective_bench(bench_t *bench)
{
    int i, nruns, ncorrectruns, paramno, paramval, nparamvals = 0;
    double *exectime = NULL, exectime_nosync;
    stats_sample_t **procstat = NULL;
    double benchtime;

    benchtime = hpctimer_wtime();
    timeslot_initialize();

    if (mpiperf_genprocreport) {
    	/* Prepare per process report */
        nparamvals = bench->paramseq_getsize(bench);
        procstat = xmalloc(sizeof(*procstat) * nparamvals);
        for (i = 0; i < nparamvals; i++) {
            procstat[i] = NULL;
        }
    }

    report_write_header(bench);
    if (mpiperf_synctype == MPIPERF_SYNC_TIME) {
    	report_write_colltest_synctime_header(bench);
    } else {
    	report_write_colltest_nosync_header(bench);
    }

    /*
     * Measure execution time of the collective for each parameter value
     * in the sequence (for example, for each message size).
     */
    paramno = 0;
    for (paramval = bench->paramseq_getcurrent(bench); paramval > 0;
         paramval = bench->paramseq_getnext(bench))
    {
        logger_log("Test %d is started; parameter value: %d", paramno, paramval);
        logger_flush();
        MPI_Barrier(MPI_COMM_WORLD);
        if (bench->getcomm(bench) != MPI_COMM_NULL) {
        	if (mpiperf_synctype == MPIPERF_SYNC_TIME) {
	        	if (mpiperf_genprocreport) {
    	        	procstat[paramno] = stats_sample_create();
                	run_collective_test_synctime(bench, &exectime, &nruns,
                		                     	 &ncorrectruns, procstat[paramno]);
                } else {
                	run_collective_test_synctime(bench, &exectime, &nruns,
                		                     	 &ncorrectruns, NULL);
                }
                report_write_colltest_synctime(bench, exectime, nruns, ncorrectruns);
            } else {
	        	if (mpiperf_genprocreport) {
    	        	procstat[paramno] = stats_sample_create();
					run_collective_test_nosync(bench, &exectime_nosync,
							                   procstat[paramno], &nruns);
	        	} else {
					run_collective_test_nosync(bench, &exectime_nosync, NULL, &nruns);
	        	}
	            report_write_colltest_nosync(bench, exectime_nosync, nruns);
            }
            free(exectime);
            exectime = NULL;
        }
        paramno++;
    }

    /* Save per process report */
    if (mpiperf_genprocreport) {
    	if (mpiperf_synctype == MPIPERF_SYNC_TIME) {
    		report_write_process_stat_synctime(bench, procstat);
        } else {
    		report_write_process_stat_nosync(bench, procstat);
        }
        for (i = 0; i < nparamvals; i++) {
	        stats_sample_free(procstat[i]);
    	}
    	free(procstat);
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
 * run_collective_test_synctime: Measures execution time of collective operation
 *     for given parameter value (send count, comm. size, etc.).
 *
 * Exectution time of collective operation is maximum time off all processes.
 * We use modification of time window-based approach for benchmarking collectives [*].
 *
 * [*] Thomas Worsch, Ralf Reussner, Werner Augustin. On Benchmarking Collective
 *     MPI Operations // In Proc. of PVM/MPI, 2002, pp. 271-279.
 */
int run_collective_test_synctime(bench_t *bench, double **exectime,
		                         int *nmeasurements, int *ncorrect_measurements,
		                         stats_sample_t *procstat)
{
    int i, stage = 0, stage_nruns, nruns = 0, ncorrectruns = 0;
    int exectime_size = 0, nerrors = 0;
    double *stage_exectime = NULL, *exectime_reduced = NULL;
    double stagetime, stagetime_max, slotlen;
    stats_sample_t *exectime_stat;
    
    /* Initialize benchmark before the test */
    if (bench->init_test) {
        bench->init_test(bench);
    }

    if ( (exectime_stat = stats_sample_create()) == NULL) {
        exit_error("No enough memory");
    }

    stage_nruns = TEST_STAGE_NRUNS_INIT;
    stage_exectime = xrealloc(stage_exectime,
                              sizeof(*stage_exectime) * TEST_STAGE_NRUNS);
    slotlen = 0.0;
    timeslot_initialize_test(bench);

    for (stage = -1; ; stage++) {

        logger_flush();
        timeslot_set_length(slotlen);
        timeslot_set_starttime(bench);
        stagetime = hpctimer_wtime();

        /* Run measurements */
        for (i = 0; i < stage_nruns; i++) {
            stage_exectime[i] = bench->measure_sync(bench);
        }

        stagetime = hpctimer_wtime() - stagetime;
        MPI_Allreduce(&stagetime, &stagetime_max, 1, MPI_DOUBLE, MPI_MAX,
                      bench->getcomm(bench));

        if (stage == -1) {
            /* Warmup stage */
            slotlen = stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE;
            stage_nruns = TEST_STAGE_NRUNS;
            logger_log("Warmup stage is finished: %d runs, slotlen = %.6f",
                       TEST_STAGE_NRUNS_INIT, slotlen);
            continue;
        }

        nruns += stage_nruns;

        /* Gather results of previous stage */
        exectime_reduced = xrealloc(exectime_reduced,
            sizeof(*exectime_reduced) * stage_nruns);

        MPI_Allreduce(stage_exectime, exectime_reduced, stage_nruns,
                      MPI_DOUBLE, MPI_MAX, bench->getcomm(bench));

        /* Allocate memory for stage results */
        if (exectime_size < (ncorrectruns + stage_nruns)) {
            if (exectime_size > 0) {
                exectime_size *= TEST_REALLOC_GROWSTEP;
            } else {
                exectime_size = stage_nruns;
            }
            *exectime = xrealloc(*exectime, sizeof(**exectime) * exectime_size);
        }
        nerrors = 0;
        for (i = 0; i < stage_nruns; i++) {
            if (!IS_MEASURE_INVALID(exectime_reduced[i])) {
                /* Add result to global list */
                (*exectime)[ncorrectruns++] = exectime_reduced[i];
                stats_sample_add(exectime_stat, exectime_reduced[i]);
                if (procstat) {
                    stats_sample_add(procstat, stage_exectime[i]);
                }
                logger_log("Measured time (stage %d, run %d): %.6f",
                           stage, i, stage_exectime[i]);
            } else {
                /*
                 * Some of processes at the measure #i
                 * was started after established time or terminated
                 * after time slot deadline.
                 */
                nerrors++;
            }
        } /* for stage_nruns */

        logger_log("Stage %d is finished: %d runs, %d invalid runs, RSE = %.2f",
                   stage, stage_nruns, nerrors,
                   stats_sample_stderr_rel(exectime_stat));

        /* Check exit condition */
        if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
            if ((ncorrectruns >= mpiperf_nmeasures_max ||
                nruns >= mpiperf_nruns_max) && (nruns >= mpiperf_nruns_min))
            {
                break;
            }
        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
            if (((stats_sample_stderr_rel(exectime_stat) <= mpiperf_rse_max) &&
                 ncorrectruns >= mpiperf_nruns_min) || (nruns >= mpiperf_nruns_max))
            {
                break;
            }
        }

        /* Adjust timeslot parameters */
        if (nerrors > stage_nruns / 4.0) {
            slotlen = stats_fmax2(2.0 * slotlen,
                                  stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE);
            logger_log("Corrected timeslot length: %.6f", slotlen);
        }

    } /* stages */

    *nmeasurements = nruns;
    *ncorrect_measurements = ncorrectruns;

    logger_log("Test is finished: %d runs, %d correct runs, RSE = %.2f",
               nruns, ncorrectruns, stats_sample_stderr_rel(exectime_stat));

    free(exectime_reduced);
    free(stage_exectime);
    stats_sample_free(exectime_stat);

    if (bench->free_test) {
        bench->free_test(bench);
    }

    return MPIPERF_SUCCESS;
}

/*
 * run_collective_test_nosync: Measures execution time of collective operation
 *                             for given parameter value.
 *
 * This test implements pipelined measurements (without synchronization of runs).
 */
int run_collective_test_nosync(bench_t *bench, double *exectime,
		                       stats_sample_t *procstat, int *nmeasurements)
{
	int i;

    /* Initialize benchmark before the test */
    if (bench->init_test) {
        bench->init_test(bench);
    }

    /* Warmup */
	bench->measure_sync(bench);
    MPI_Barrier(bench->getcomm(bench));

	/* Run measurements */
    *exectime = hpctimer_wtime();
    for (i = 0; i < mpiperf_nruns_max; i++) {
    	bench->measure_sync(bench);
    }
    /* Average time of one measure in current process */
    *exectime = (hpctimer_wtime() - *exectime) / mpiperf_nruns_max;
    *nmeasurements = mpiperf_nruns_max;

    if (procstat) {
        stats_sample_add(procstat, *exectime);
    }

    if (bench->free_test) {
        bench->free_test(bench);
    }

    return MPIPERF_SUCCESS;
}

/*
 * run_pt2pt_bench: Runs benchmark for point-to-point operation.
 *                  Benchmark is a sequence of runs for each value of variable
 *                  parameter (commsize, send count, etc).
 */
int run_pt2pt_bench(bench_t *bench)
{
    int nruns, ncorrectruns, paramno, paramval;
    double *exectime = NULL;
    double benchtime;

    benchtime = hpctimer_wtime();
    timeslot_initialize();

    report_write_header(bench);
    report_write_pt2pttest_header(bench);

    /*
     * Measure execution time of the operation for each parameter value
     * in the sequence.
     */
    paramno = 0;
    for (paramval = bench->paramseq_getcurrent(bench); paramval > 0;
         paramval = bench->paramseq_getnext(bench))
    {
        logger_log("Test %d is started; parameter value: %d", paramno, paramval);
        logger_flush();
        MPI_Barrier(MPI_COMM_WORLD);
        if (bench->getcomm(bench) != MPI_COMM_NULL) {
            run_pt2pt_test_synctime(bench, &exectime, &nruns, &ncorrectruns);
            report_write_pt2pttest_exectime(bench, exectime, nruns,
                                            ncorrectruns);
            free(exectime);
            exectime = NULL;
        }
        paramno++;
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
 * run_pt2pt_test: Measures execution time of point-to-point operation for
 *                 given parameter value (send count, comm. size, etc.).
 *
 * Exectution time of point-to-point operation is a time of master process.
 */
int run_pt2pt_test_synctime(bench_t *bench, double **exectime, int *nmeasurements,
                            int *ncorrect_measurements)
{
    int i, stage = 0, stage_nruns, nruns = 0, ncorrectruns = 0;
    int exectime_size = 0, nerrors = 0;
    double *stage_exectime = NULL;
    double stagetime, stagetime_max, slotlen;
    stats_sample_t *exectime_stat = NULL;
    int breakflag;

    /* Initialize benchmark before the test */
    if (bench->init_test) {
        bench->init_test(bench);
    }

    if (IS_MASTER_RANK) {
        if ( (exectime_stat = stats_sample_create()) == NULL) {
            exit_error("No enough memory");
        }
    }

    stage_nruns = TEST_STAGE_NRUNS_INIT;
    stage_exectime = xrealloc(stage_exectime,
                              sizeof(*stage_exectime) * TEST_STAGE_NRUNS);
    slotlen = 0.0;
    timeslot_initialize_test(bench);

    for (stage = -1; ; stage++) {

        timeslot_set_length(slotlen);
        timeslot_set_starttime(bench);
        stagetime = hpctimer_wtime();

        /* Run measurements */
        for (i = 0; i < stage_nruns; i++) {
            stage_exectime[i] = bench->measure_sync(bench);
        }

        stagetime = hpctimer_wtime() - stagetime;
        MPI_Allreduce(&stagetime, &stagetime_max, 1, MPI_DOUBLE, MPI_MAX,
                      bench->getcomm(bench));

        if (stage == -1) {
            /* Warmup stage is finished. Setup next timeslot. */
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
                if (exectime_size > 0) {
                    exectime_size *= TEST_REALLOC_GROWSTEP;
                } else {
                    exectime_size = stage_nruns;
                }
                *exectime = xrealloc(*exectime, sizeof(**exectime) * exectime_size);
            }
            nerrors = 0;
            for (i = 0; i < stage_nruns; i++) {
                if (!IS_MEASURE_INVALID(stage_exectime[i])) {
                    /* Add result to global list */
                    (*exectime)[ncorrectruns++] = stage_exectime[i];
                    stats_sample_add(exectime_stat, stage_exectime[i]);
                    logger_log("Measured time (stage %d, run %d): %.6f",
                               stage, i, stage_exectime[i]);
                } else {
                    /*
                     * Some of processes at the measure #i
                     * was started after established time or terminated
                     * after time slot deadline.
                     */
                    nerrors++;
                }
            } /* for stage_nruns */

            logger_log("Stage %d is finished: %d runs, %d invalid runs, RSE = %.2f",
                       stage, stage_nruns, nerrors,
                       stats_sample_stderr_rel(exectime_stat));

            /* Check exit condition */
            breakflag = 0;
            if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
                if ((ncorrectruns >= mpiperf_nmeasures_max ||
                    nruns >= mpiperf_nruns_max) && (nruns >= mpiperf_nruns_min))
                {
                    breakflag = 1;
                }
            } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
                if (((stats_sample_stderr_rel(exectime_stat) <= mpiperf_rse_max) &&
                     ncorrectruns >= mpiperf_nruns_min) || (nruns >= mpiperf_nruns_max))
                {
                    breakflag = 1;
                }
            }
        } /* master process */

        /* Check exit condition */
        MPI_Bcast(&breakflag, 1, MPI_INT, mpiperf_master_rank,
                  bench->getcomm(bench));
        if (breakflag) {
            break;
        }

        /* Adjust timeslot parameters */
        MPI_Bcast(&nerrors, 1, MPI_INT, mpiperf_master_rank,
                  bench->getcomm(bench));
        if (nerrors > stage_nruns / 4.0) {
            slotlen = stats_fmax2(2.0 * slotlen,
                                  stagetime_max / stage_nruns * TEST_SLOTLEN_SCALE);
            logger_log("Corrected timeslot length: %.6f", slotlen);
        }
    } /* stages */

    *nmeasurements = nruns;
    *ncorrect_measurements = ncorrectruns;
    if (IS_MASTER_RANK) {
        logger_log("Test is finished: %d runs, %d correct runs, RSE = %.2f",
                   nruns, ncorrectruns, stats_sample_stderr_rel(exectime_stat));
    }

    free(stage_exectime);
    if (IS_MASTER_RANK) {
        stats_sample_free(exectime_stat);
    }
    if (bench->free_test) {
        bench->free_test(bench);
    }

    return MPIPERF_SUCCESS;
}

/* is_pt2pt_benchmark: */
int is_pt2pt_benchmark(bench_t *bench)
{
    return bench->mpifunctype == MPIFUNC_TYPE_PT2PT;
}

/* getcommworld: */
MPI_Comm getcommworld(bench_t *bench)
{
    return MPI_COMM_WORLD;
}

/* getcommworld_size: */
int getcommworld_size(bench_t *bench)
{
    return mpiperf_commsize;
}

/* getcommworld_rank: */
int getcommworld_rank(bench_t *bench)
{
    return mpiperf_rank;
}

