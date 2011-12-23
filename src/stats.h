/*
 * stats.h: Statistical functions.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef STATS_H
#define STATS_H

enum StatsConfidenceLevels {
    STATS_CONFIDENCE_LEVEL_90 = 0,
    STATS_CONFIDENCE_LEVEL_95 = 1,
    STATS_CONFIDENCE_LEVEL_99 = 2
};

typedef struct stats_sample stats_sample_t;

stats_sample_t *stats_sample_create();
void stats_sample_free(stats_sample_t *sample);
void stats_sample_add(stats_sample_t *sample, double val);
void stats_sample_add_dataset(stats_sample_t *sample, double *dataset, int size);
void stats_sample_clean(stats_sample_t *sample);

double stats_sample_mean(stats_sample_t *sample);
double stats_sample_var(stats_sample_t *sample);
double stats_sample_stddev(stats_sample_t *sample);
double stats_sample_stderr(stats_sample_t *sample);
double stats_sample_stderr_rel(stats_sample_t *sample);
double stats_sample_min(stats_sample_t *sample);
double stats_sample_max(stats_sample_t *sample);
int stats_sample_min_index(stats_sample_t *sample);
int stats_sample_max_index(stats_sample_t *sample);
int stats_sample_size(stats_sample_t *sample);

double stats_mean(double *data, int size);
double stats_var(double *data, int size);
double stats_stddev(double *data, int size);
double stats_stderr(double *data, int size);
double stats_stderr_rel(double *data, int size);
double stats_min(double *data, int size);
int stats_min_index(double *data, int size);
double stats_max(double *data, int size);
int stats_max_index(double *data, int size);

double stats_fmax2(double a, double b);
double stats_fmin2(double a, double b);

int stats_dataset_remove_extreme(double *data, int size, int lb, int ub);
int stats_sample_confidence_interval(stats_sample_t *sample, int level,
                                     double *lb, double *ub, double *err);
#endif /* STATS_H */
