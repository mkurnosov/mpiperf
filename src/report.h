/*
 * report.h: Report module.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#include <mpi.h>

#ifndef REPORT_H
#define REPORT_H

#include "bench.h"
#include "stats.h"

int report_write_header(bench_t *bench);

int report_write_colltest_synctime_header(bench_t *bench);
int report_write_colltest_nosync_header(bench_t *bench);

int report_write_pt2pttest_header(bench_t *bench);

int report_write_colltest_synctime(bench_t *bench, double *exectime_local,
                                   int nruns, int ncorrectruns);
int report_write_colltest_nosync(bench_t *bench, double exectime_local, int nruns);

int report_write_pt2pttest_exectime(bench_t *bench, double *exectime,
                                   int nruns, int ncorrectruns);

int report_write_process_stat_synctime(bench_t *bench, stats_sample_t **procstat);
int report_write_process_stat_nosync(bench_t *bench, stats_sample_t **procstat);

void report_printf(const char *format, ...);

#endif /* REPORT_H */
