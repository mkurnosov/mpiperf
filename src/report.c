/*
 * report.c:
 *
 * Copyright (C) 2011-2012 Mikhail Kurnosov
 */

#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "report.h"
#include "mpiperf.h"
#include "stat.h"
#include "util.h"
#include "hpctimer.h"

int report_write_header()
{
    int i, namelen;
    char procname[MPI_MAX_PROCESSOR_NAME];

    if (IS_MASTER_RANK) {
        printf("# mpiperf %d.%d.%d report\n", MPIPERF_VERSION_MAJOR,
                MPIPERF_VERSION_MINOR, MPIPERF_VERSION_PATCH);
        printf("# Command line: %s\n", mpiperf_cmdline);
        printf("# Benchmark: %s\n", mpiperf_benchname);
        printf("# Min number of processes: %d\n", mpiperf_nprocs_min);
        printf("# Max number of processes: %d\n", mpiperf_nprocs_max);
        printf("# Processes step: %d\n", mpiperf_nprocs_step);
        printf("# Min data size: %d\n", mpiperf_count_min);
        printf("# Max data size: %d\n", mpiperf_count_max);
        printf("# Data size step: %d\n", mpiperf_count_step);

        if (mpiperf_test_exit_cond == TEST_EXIT_COND_NRUNS) {
            printf("# Test exit condition: %d successful runs\n",
                   mpiperf_nmeasures_max);
        } else if (mpiperf_test_exit_cond == TEST_EXIT_COND_STDERR) {
            printf("# Test exit condition: relative stdandard error of measurements <= %.2f\n",
                   mpiperf_rse_max);
        }

        printf("# Minimal number of runs: %d\n", mpiperf_nruns_min);
        printf("# Maximal number of runs: %d\n", mpiperf_nruns_max);
        printf("# Statistical analysis of data (removing outliers): %s\n",
               mpiperf_statanalysis ? "on" : "off");
        printf("# Compute time iterations: %d\n", mpiperf_comptime_niters);
        printf("# Cache defeat: %s\n", mpiperf_isflushcache ? "on" : "off");
        printf("# Timer: %s\n", mpiperf_timername);
        if (mpiperf_timescale == TIMESCALE_SEC) {
            printf("# Time scale: seconds\n");
        } else {
            printf("# Time scale: microseconds\n");
        }
        if (mpiperf_synctype == SYNC_TIME) {
            printf("# Synchronization method: synctime\n");
        } else {
            printf("# Synchronization method: nosync\n");
        }
        if (mpiperf_perprocreport) {
            printf("# Per process report file: %s\n", mpiperf_repfile);
        }
        if (mpiperf_logfile) {
            printf("# Log file: %s\n", mpiperf_logfile);
            printf("# Log master only: %s\n", mpiperf_logmaster_only ? "on" : "off");
        }

        printf("# Master process: %d\n", mpiperf_master_rank);
        printf("# Process mapping: \n");
        for (i = 0; i < mpiperf_commsize; i++) {
            if (i != mpiperf_master_rank) {
                MPI_Recv(procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i,
                         REPORT_MSGTAG, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
            } else {
                MPI_Get_processor_name(procname, &namelen);
            }
            printf("# %d:%s\n", i, procname);
        }
        printf("#\n");
    } else {
        MPI_Get_processor_name(procname, &namelen);
        MPI_Send(procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                 mpiperf_master_rank, REPORT_MSGTAG, MPI_COMM_WORLD);
    }
    return MPIPERF_SUCCESS;
}

/* report_printf: Prints an message to the report. */
void report_printf(const char *format, ...)
{
    va_list ap;
    static char buf[REPORT_BUFSIZE];

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    printf("%s", buf);
}

