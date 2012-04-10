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

#include "bench.h"
#include "mpiperf.h"
#include "seq.h"
#include "stats.h"
#include "timeslot.h"
#include "report.h"
#include "logger.h"
#include "util.h"
#include "hpctimer.h"

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
