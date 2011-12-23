/*
 * mpiperf.c: MPI Performance Benchmark.
 * 
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <mpi.h>

#include "mpiperf.h"
#include "report.h"
#include "timeslot.h"
#include "bench.h"
#include "mpigclock.h"
#include "hpctimer.h"
#include "logger.h"
#include "mempool.h"
#include "util.h"

enum {
    MPIPERF_CMDLINE_MAX = 1024
};

const char *mpiperf_progname = "mpiperf";

int mpiperf_rank;              /* Rank of current process in MPI_COMM_WORLD */
int mpiperf_commsize;          /* Size of MPI_COMM_WORLD */
int mpiperf_master_rank = 0;   /* Process which generates report */
char *mpiperf_cmdline = NULL;

int mpiperf_isflushcache = 0;  /* Cache defeat flag */
int mpiperf_genprocreport = 0; /* Generate per process report flag */
int mpiperf_statanalysis = 1;  /* Postprocess statistical analysis flag */
int mpiperf_timescale = MPIPERF_TIMESCALE_USEC;
int mpiperf_synctype = MPIPERF_SYNC_TIME;

char *mpiperf_timername = "MPI_Wtime";

/* Variable parameter */
int mpiperf_param_step_type = PARAM_STEP_MUL;
int mpiperf_param_min = -1;
int mpiperf_param_max = -1;
int mpiperf_param_step = -1;

/* Test's exit condition */
int mpiperf_test_exit_cond = TEST_EXIT_COND_NRUNS;
int mpiperf_nmeasures_max = 30;  /* Maximal number of successful measurements */
double mpiperf_rse_max = 0.05;   /* Maximal value of relative standard error */
int mpiperf_nruns_min = 10;      /* Minimal number of runs per test */
int mpiperf_nruns_max = 100;     /* Maximal number of runs per test */

static char *mpiperf_logfile = NULL;
static int mpiperf_logmaster_only = 0;
static bench_t *mpiperf_bench = NULL;

static void mpiperf_checktimer();
static void print_usage(int argc, char **argv);
static int parse_options(int argc, char **argv);

int main(int argc, char **argv)
{
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiperf_commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiperf_rank);

    /* Save command line */
    mpiperf_cmdline = (char *)malloc(sizeof(*mpiperf_cmdline) *
                                     MPIPERF_CMDLINE_MAX);
    if (mpiperf_cmdline) {
        strcpy(mpiperf_cmdline, argv[0]);
        for (i = 1; i < argc; i++) {
            strcat(mpiperf_cmdline, " ");
            strcat(mpiperf_cmdline, argv[i]);
        }
    }

    if (parse_options(argc, argv) == MPIPERF_FAILURE) {
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    mpiperf_initialize();
    if (is_pt2pt_benchmark(mpiperf_bench)) {
        /* Point-to-point benchmark */
    	run_pt2pt_bench(mpiperf_bench);
    } else {
        /* Collective benchmark */
   		run_collective_bench(mpiperf_bench);
    }
    mpiperf_finalize();

    free(mpiperf_cmdline);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

/* mpiperf_initalize: */
void mpiperf_initialize()
{
    int rc;

    rc = logger_initialize(mpiperf_logfile, mpiperf_logmaster_only);
    if (rc == MPIPERF_FAILURE) {
        exit_error("Logger initialization failed");
    }

    if (mpiperf_timername != NULL) {
        if (hpctimer_initialize(mpiperf_timername) == HPCTIMER_FAILURE) {
            exit_error("Error initializing timer: %s", mpiperf_timername);
        }
        logger_log("hpctimer '%s' is initialized", mpiperf_timername);
    }

    rc = bench_initialize(mpiperf_bench);
    if (rc == MPIPERF_FAILURE) {
        exit_error("Error initializing benchmark");
    }
}

/* mpiperf_finalize: */
void mpiperf_finalize()
{
    bench_finalize(mpiperf_bench);
    logger_finalize();
    hpctimer_finalize();
}

/* mpiperf_checktimer: */
static void mpiperf_checktimer()
{
    int sanity = 1, *sanityall = NULL;

    if (mpiperf_timername != NULL) {
        if (hpctimer_initialize(mpiperf_timername) == HPCTIMER_FAILURE) {
            exit_error("Error initializing timer: %s", mpiperf_timername);
        }
    }

    if (IS_MASTER_RANK) {
        if ( (sanityall = malloc(sizeof(*sanityall) * mpiperf_commsize)) == NULL) {
            exit_error("No enough memory");
        }
    }
    sanity = hpctimer_sanity_check();

    MPI_Gather(&sanity, 1, MPI_INT, sanityall, 1, MPI_INT, mpiperf_master_rank,
               MPI_COMM_WORLD);
    if (IS_MASTER_RANK) {
        printf("# mpiperf timer '%s' sanity checking:\n", mpiperf_timername);
        for (int i = 0; i < mpiperf_commsize; i++) {
            printf("#   process %d: %s\n",
                   i, sanityall[i] ? "PASSED" : "FAILED");
            sanity = sanity && sanityall[i];
        }
        printf("# mpiperf timer '%s' sanity check: %s\n", mpiperf_timername,
               sanity ? "PASSED" : "FAILED");
    }
    if (IS_MASTER_RANK) {
        free(sanityall);
    }
    hpctimer_finalize();
}

/* print_usage: Prints usage. */
static void print_usage(int argc, char **argv)
{
    /*
     * TODO:
     * - add long options
     */
    fprintf(stderr, "Usage: %s [OPTIONS] BENCHMARK\n", mpiperf_progname);
    fprintf(stderr, "Measure performance of MPI routines by BENCHMARK.\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -x <value>       Lower bound of variable parameter value\n");
    fprintf(stderr, "  -X <value>       Upper bound of variable parameter value\n");
    fprintf(stderr, "  -s <step>        Step value in parameter sequence: arithmetic progression with <step>\n");
    fprintf(stderr, "  -S <step>        Step value in parameter sequence: geometric progression with <step> (default)\n");
    fprintf(stderr, "  -e <rse%%>        Terminate each test when relative standard error is less then <rse> (default: %d%%)\n",
            (int)(mpiperf_rse_max * 100));
    fprintf(stderr, "  -E <n>           Terminate each test when number of successful measurements is equal to or greater then <n> (default: %d)\n",
            mpiperf_nmeasures_max);
    fprintf(stderr, "  -r <runs>        Minimal number of runs for each parameter value (default: %d)\n",
            mpiperf_nruns_min);
    fprintf(stderr, "  -R <runs>        Maximal number of runs for each parameter value (default: %d)\n",
            mpiperf_nruns_max);
    fprintf(stderr, "  -z               Synchronization method: synctime, nosync (default: synctime)\n");
    fprintf(stderr, "  -p               Generate additional report per process (default: off)\n");
    fprintf(stderr, "  -a               Turn off statistical analysis of data (removing outliers, default: on)\n");
    fprintf(stderr, "  -w <scale>       Time scale: sec, usec (deafult: usec)\n");
    fprintf(stderr, "  -c               CPU cache defeat flag (default: off)\n");
    fprintf(stderr, "  -t               Timer: tsc, mpi_wtime, gettimeofday (default: mpi_wtime)\n");
    fprintf(stderr, "  -T               Display list of supported timers\n");
    fprintf(stderr, "  -g               Run sanity check of timer\n");
    fprintf(stderr, "  -l <file>        Log filename\n");
    fprintf(stderr, "  -m               Log by master process only (default: off)\n");
    fprintf(stderr, "  -q               Display information about all benchmarks\n");
    fprintf(stderr, "  -h               Display this information\n");
    fprintf(stderr, "\nReport bugs to <mkurnosov@gmail.com>.\n");
}

/* parse_options: Parses command line options and sets global variables. */
static int parse_options(int argc, char **argv)
{
    int opt, checktimer = 0;

    while ( (opt = getopt(argc, argv, "apchqTgx:X:r:R:s:S:e:E:l:t:w:z:m")) != -1) {
        switch (opt) {
        case 'a':
            mpiperf_statanalysis = 0;
            break;
        case 'c':
            mpiperf_isflushcache = 1;
            break;
        case 'x':
            mpiperf_param_min = parse_intval(optarg);
            break;
        case 'X':
            mpiperf_param_max = parse_intval(optarg);
            break;
        case 'e':
            mpiperf_test_exit_cond = TEST_EXIT_COND_STDERR;
            mpiperf_rse_max = atoi(optarg) / 100.0;
            if (mpiperf_rse_max < 1E-3) {
                if (IS_MASTER_RANK) {
                    exit_error("Incorrect value of RSE (-e)");
                }
            }
            break;
        case 'E':
            mpiperf_test_exit_cond = TEST_EXIT_COND_NRUNS;
            mpiperf_nmeasures_max = atoi(optarg);
            if (mpiperf_nmeasures_max < 1) {
                exit_error("Incorrect value of number of measurements (-E)");
            }
            break;
        case 'r':
            mpiperf_nruns_min = atoi(optarg);
            if (mpiperf_nruns_min < 1) {
                exit_error("Incorrect value of minimal number of runs (-r)");
            }
            break;
        case 'R':
            mpiperf_nruns_max = atoi(optarg);
            if (mpiperf_nruns_max < 1 || mpiperf_nruns_max < mpiperf_nruns_min) {
                exit_error("Incorrect value of maximal number of runs (-R)");
            }
            break;
        case 's':
            mpiperf_param_step = atoi(optarg);
            mpiperf_param_step_type = PARAM_STEP_INC;
            break;
        case 'S':
            mpiperf_param_step = atoi(optarg);
            mpiperf_param_step_type = PARAM_STEP_MUL;
            break;
        case 'p':
            mpiperf_genprocreport = 1;
            break;
        case 'l':
            mpiperf_logfile = optarg;
            break;
        case 'z':
            if (strcasecmp(optarg, "synctime") == 0) {
                mpiperf_synctype = MPIPERF_SYNC_TIME;
            } else if (strcasecmp(optarg, "nosync") == 0) {
                mpiperf_synctype = MPIPERF_SYNC_NONE;
            } else {
                exit_error("Unknown synchronization method: %s", optarg);
            }
            break;
        case 'w':
            if (strcasecmp(optarg, "usec") == 0) {
                mpiperf_timescale = MPIPERF_TIMESCALE_USEC;
            } else if (strcasecmp(optarg, "sec") == 0) {
                mpiperf_timescale = MPIPERF_TIMESCALE_SEC;
            } else {
                exit_error("Unknown time scale: %s", optarg);
            }
            break;
        case 't':
            mpiperf_timername = optarg;
            break;
        case 'T':
            if (IS_MASTER_RANK) {
                hpctimer_print_timers();
            }
            exit_success();
        case 'g':
        	checktimer = 1;
            break;
        case 'm':
            mpiperf_logmaster_only = 1;
            break;
        case 'q':
            if (IS_MASTER_RANK) {
                print_benchmarks_info();
            }
            exit_success();
        case 'h':
        default:
            if (IS_MASTER_RANK) {
                print_usage(argc, argv);
            }
            exit_success();
        }
    }

	if (checktimer) {
		mpiperf_checktimer();
    	exit_success();
	}

    if (optind >= argc) {
        if (IS_MASTER_RANK) {
            print_usage(argc, argv);
            print_error("Expected benchmark name");
        }
        return MPIPERF_FAILURE;
    }

    if ( (mpiperf_bench = lookup_bench(argv[optind])) == NULL) {
        if (IS_MASTER_RANK) {
            print_error("Unknown benchmark name");
        }
        return MPIPERF_FAILURE;
    }
    return MPIPERF_SUCCESS;
}
