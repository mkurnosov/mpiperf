/*
 * mpiperf.c: MPI Performance Benchmark.
 * 
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <mpi.h>

#include "mpiperf.h"
#include "version.h"
#include "report.h"
#include "timeslot.h"
#include "mpigclock.h"
#include "hpctimer.h"
#include "logger.h"
#include "mempool.h"
#include "util.h"
#include "bench_coll.h"
#include "bench_pt2pt.h"
#include "bench_nbc.h"

enum Consts {
    MPIPERF_CMDLINE_MAX = 1024
};

int mpiperf_rank;           /* Rank of current process in MPI_COMM_WORLD */
int mpiperf_commsize;       /* Size of MPI_COMM_WORLD */
int mpiperf_master_rank;    /* Process which generates report */
char *mpiperf_cmdline;
char *mpiperf_progname;

/*
 * Command-line options
 */
int mpiperf_nprocs_min;
int mpiperf_nprocs_max;
int mpiperf_nprocs_step_type;
int mpiperf_nprocs_step;

int mpiperf_count_min;
int mpiperf_count_max;
int mpiperf_count_step_type;
int mpiperf_count_step;

int mpiperf_perprocreport;
char *mpiperf_repfile;
FILE *mpiperf_repstream;

int mpiperf_isflushcache;
int mpiperf_statanalysis;
int mpiperf_timescale;
int mpiperf_synctype;
char *mpiperf_timername;

int mpiperf_test_exit_cond;
int mpiperf_nmeasures_max;
double mpiperf_rse_max;
int mpiperf_nruns_min;
int mpiperf_nruns_max;

char *mpiperf_logfile;
int mpiperf_logmaster_only;

int mpiperf_confidence_level_type;
int mpiperf_confidence_level;

int mpiperf_nbcbench_mode;
int mpiperf_comptime_niters;

char *mpiperf_benchname = NULL;

int mpiperf_is_measure_started;

static collbench_t *mpiperf_collbench = NULL;
static pt2ptbench_t *mpiperf_pt2ptbench = NULL;
static nbcbench_t *mpiperf_nbcbench = NULL;

static void mpiperf_checktimer();
static void print_version();
static void print_usage(int argc, char **argv);
static int parse_options(int argc, char **argv);
static void set_default_options();

int main(int argc, char **argv)
{
    int i;

    /* This flag can be used in MPI_Init by user library. */
    mpiperf_is_measure_started = 0;

    MPI_Init(&argc, &argv);
    set_default_options();

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
    if (mpiperf_collbench) {
        run_collbench(mpiperf_collbench);
    } else if (mpiperf_pt2ptbench) {
        run_pt2ptbench(mpiperf_pt2ptbench);
    } else if (mpiperf_nbcbench) {
        run_nbcbench(mpiperf_nbcbench);
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

    if (mpiperf_perprocreport) {
        if ( (mpiperf_repstream = fopen(mpiperf_repfile, "w")) == NULL)
            exit_error("Can't open per process report file: %s", optarg);
    }
}

/* mpiperf_finalize: */
void mpiperf_finalize()
{
    if (mpiperf_perprocreport)
        fclose(mpiperf_repstream);

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

/* print_version: Prints the version number. */
static void print_version()
{
    fprintf(stdout, "%s %d.%d.%d\n", mpiperf_progname, MPIPERF_VERSION_MAJOR, 
                                     MPIPERF_VERSION_MINOR, MPIPERF_VERSION_PATCH);
}

/* print_usage: Prints usage. */
static void print_usage(int argc, char **argv)
{
    fprintf(stderr, "Usage: %s [OPTIONS] BENCHMARK\n", mpiperf_progname);
    fprintf(stderr, "Measure performance of MPI routines by BENCHMARK.\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -p <value>       Min number of processes (default: commsize)\n");
    fprintf(stderr, "  -P <value>       Max number of processes (defaul: commsize)\n");
    fprintf(stderr, "  -x <value>       Min data size (count elems) (defaul: 1)\n");
    fprintf(stderr, "  -X <value>       Max data size (count elems) (defaul: 100)\n");
    fprintf(stderr, "  -g <step>        Number of processes is changed in an arithmetic progression with <step>\n");
    fprintf(stderr, "  -G <step>        Number of processes is changed in a geometric progression with <step> (defaul: 2)\n");
    fprintf(stderr, "  -s <step>        Data size is changed in an arithmetic progression with <step>\n");
    fprintf(stderr, "  -S <step>        Data size is changed in a geometric progression with <step> (defaul: 2)\n");
    fprintf(stderr, "  -e <rse%%>        Terminate each test when relative standard error is less then <rse> (default: %d%%)\n",
            (int)(mpiperf_rse_max * 100));
    fprintf(stderr, "  -E <n>           Terminate each test when number of successful measurements is equal to or greater then <n> (default: %d)\n",
            mpiperf_nmeasures_max);
    fprintf(stderr, "  -r <runs>        Minimal number of runs for each parameter value (default: %d)\n",
            mpiperf_nruns_min);
    fprintf(stderr, "  -R <runs>        Maximal number of runs for each parameter value (default: %d)\n",
            mpiperf_nruns_max);
    fprintf(stderr, "  -b               Measure NBC time in blocking mode (default: off)\n");
    fprintf(stderr, "  -c               Number of iterations for compute time (on NBC overlap measuring, default: 10)\n");
    fprintf(stderr, "  -z               Synchronization method: synctime, nosync (default: synctime)\n");
    fprintf(stderr, "  -o <file>        Write per process report in <file> (default: off)\n");
    fprintf(stderr, "  -a               Turn off statistical analysis of data (removing outliers, default: on)\n");
    fprintf(stderr, "  -w <scale>       Time scale: sec, usec (default: usec)\n");
    fprintf(stderr, "  -d               CPU cache defeat flag (default: off)\n");
    fprintf(stderr, "  -t               Timer: tsc, mpi_wtime, gettimeofday (default: mpi_wtime)\n");
    fprintf(stderr, "  -T               Display list of supported timers\n");
    fprintf(stderr, "  -j               Run sanity check of timer\n");
    fprintf(stderr, "  -l <file>        Log filename\n");
    fprintf(stderr, "  -m               Log by master process only (default: off)\n");
    fprintf(stderr, "  -q               Display information about all benchmarks\n");
    fprintf(stderr, "  -h               Display this information\n");
    fprintf(stderr, "  -v               Display the version number\n");
    fprintf(stderr, "\nReport bugs to <mkurnosov@gmail.com>.\n");
}

/* parse_options: Parses command line options and sets global variables. */
static int parse_options(int argc, char **argv)
{
    int opt;

    while ( (opt = getopt(argc, argv, "p:P:x:X:g:G:s:S:e:E:r:R:l:z:w:t:o:c:badTjmqh")) != -1) {
        switch (opt) {
        case 'p':
            mpiperf_nprocs_min = atoi(optarg);
            break;
        case 'P':
            mpiperf_nprocs_max = atoi(optarg);
            break;
        case 'x':
            mpiperf_count_min = parse_intval(optarg);
            break;
        case 'X':
            mpiperf_count_max = parse_intval(optarg);
            break;
        case 'g':
            mpiperf_nprocs_step = atoi(optarg);
            mpiperf_nprocs_step_type = STEP_TYPE_INC;
            break;
        case 'G':
            mpiperf_nprocs_step = atoi(optarg);
            mpiperf_nprocs_step_type = STEP_TYPE_MUL;
            break;
        case 's':
            mpiperf_count_step = atoi(optarg);
            mpiperf_count_step_type = STEP_TYPE_INC;
            break;
        case 'S':
            mpiperf_count_step = atoi(optarg);
            mpiperf_count_step_type = STEP_TYPE_MUL;
            break;
        case 'b':
            mpiperf_nbcbench_mode = NBCBENCH_BLOCKING;
            break;
        case 'c':
            mpiperf_comptime_niters = atoi(optarg);
            if (mpiperf_comptime_niters < 1) {
                exit_error("Incorrect number of iterations for compute time (-c)");
            }
            break;
        case 'a':
            mpiperf_statanalysis = 0;
            break;
        case 'd':
            mpiperf_isflushcache = 1;
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
                exit_error("Incorrect number of measurements (-E)");
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
        case 'o':
            mpiperf_perprocreport = 1;
            mpiperf_repfile = optarg;
            break;
        case 'l':
            mpiperf_logfile = optarg;
            break;
        case 'z':
            if (strcasecmp(optarg, "synctime") == 0) {
                mpiperf_synctype = SYNC_TIME;
            } else if (strcasecmp(optarg, "nosync") == 0) {
                mpiperf_synctype = SYNC_NONE;
            } else {
                exit_error("Unknown synchronization method: %s", optarg);
            }
            break;
        case 'w':
            if (strcasecmp(optarg, "usec") == 0) {
                mpiperf_timescale = TIMESCALE_USEC;
            } else if (strcasecmp(optarg, "sec") == 0) {
                mpiperf_timescale = TIMESCALE_SEC;
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
        case 'j':
            mpiperf_checktimer();
            exit_success();
            break;
        case 'm':
            mpiperf_logmaster_only = 1;
            break;
        case 'q':
            if (IS_MASTER_RANK) {
                print_collbench_info();
                print_pt2ptbench_info();
                print_nbcbench_info();
            }
            exit_success();
        case 'v':
            if (IS_MASTER_RANK) {
                print_version();
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

    if (optind >= argc) {
        if (IS_MASTER_RANK) {
            print_usage(argc, argv);
            print_error("Expected benchmark name");
        }
        return MPIPERF_FAILURE;
    }

    if (mpiperf_commsize < 2) {
        MPI_Finalize();
        fprintf(stderr, "Number of processes is too small: %d\n", mpiperf_commsize);
        exit(EXIT_FAILURE);
    }

    if (mpiperf_nprocs_min < 1 || mpiperf_nprocs_max > mpiperf_commsize ||
        mpiperf_nprocs_min > mpiperf_nprocs_max)
    {
        if (IS_MASTER_RANK) {
            exit_error("Incorrect number of processes");
        }
    }

    if (mpiperf_count_min < 0 || mpiperf_count_max < 0 ||
        mpiperf_count_min > mpiperf_count_max)
    {
        if (IS_MASTER_RANK) {
            exit_error("Incorrect count (data size)");
        }
    }

    if (mpiperf_nprocs_step == 0 ||  mpiperf_count_step == 0) {
        exit_error("Incorrect step");
    }

    if (mpiperf_nprocs_step_type == STEP_TYPE_MUL) {
        if (mpiperf_nprocs_step < 2 ) {
            exit_error("Incorrect process step: %d", mpiperf_nprocs_step);
        }
    }

    if (mpiperf_count_step_type == STEP_TYPE_MUL) {
        if (mpiperf_count_step < 2 ) {
            exit_error("Incorrect count step: %d", mpiperf_count_step);
        }
    }

    if (mpiperf_count_max == 0 && mpiperf_count_step_type == STEP_TYPE_MUL) {
        mpiperf_count_step_type = STEP_TYPE_INC;
        mpiperf_count_step = 1;
    }

    /* Lookup benchmark by name */
    mpiperf_benchname = argv[optind];
    if ( (mpiperf_collbench = lookup_collbench(argv[optind])))
        return MPIPERF_SUCCESS;
    else if ( (mpiperf_pt2ptbench = lookup_pt2ptbench(argv[optind])))
        return MPIPERF_SUCCESS;
    else if ( (mpiperf_nbcbench = lookup_nbcbench(argv[optind])))
        return MPIPERF_SUCCESS;
    else {
        if (IS_MASTER_RANK) {
            print_error("Unknown benchmark name");
        }
    }
    return MPIPERF_FAILURE;
}

/* set_default_options: Setups default values for options */
static void set_default_options()
{
    mpiperf_progname = "mpiperf";
    MPI_Comm_size(MPI_COMM_WORLD, &mpiperf_commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiperf_rank);
    mpiperf_master_rank = 0;

    mpiperf_nprocs_min = mpiperf_commsize;      /* -p */
    mpiperf_nprocs_max = mpiperf_commsize;      /* -P */
    mpiperf_nprocs_step_type = STEP_TYPE_MUL;   /* -G */
    mpiperf_nprocs_step = 2;                    /* -G value */

    mpiperf_count_min = 1;                      /* -x */
    mpiperf_count_max = 128;                    /* -X */
    mpiperf_count_step_type = STEP_TYPE_MUL;    /* -S */
    mpiperf_count_step = 2;                     /* -S value */

    mpiperf_perprocreport = 0;
    mpiperf_repfile = NULL;                     /* -o */
    mpiperf_repstream = NULL;

    mpiperf_isflushcache = 0;                   /* -c Cache defeat flag */
    mpiperf_statanalysis = 1;                   /* -a Stat. analysis */
    mpiperf_timescale = TIMESCALE_USEC;         /* -w */
    mpiperf_synctype = SYNC_TIME;               /* -z Sync. method */
    mpiperf_timername = "MPI_Wtime";            /* -t Timer */

    mpiperf_test_exit_cond = TEST_EXIT_COND_NRUNS;  /* -E */
    mpiperf_nmeasures_max = 30;  /* -E Maximal number of successful measurements */
    mpiperf_rse_max = 0.05;      /* -e Maximal value of relative standard error */
    mpiperf_nruns_min = 10;      /* -r Minimal number of runs per test */
    mpiperf_nruns_max = 100;     /* -R Maximal number of runs per test */

    mpiperf_logfile = NULL;      /* -l Log filename */
    mpiperf_logmaster_only = 0;  /* -m Log by master process only */

    mpiperf_confidence_level_type = STAT_CONFIDENCE_LEVEL_95;
    mpiperf_confidence_level = 95;

    mpiperf_nbcbench_mode = NBCBENCH_OVERLAP;
    mpiperf_comptime_niters = 10;
    mpiperf_benchname = NULL;
}
