/*
 * stat.h: Statistical functions.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef STAT_H
#define STAT_H

enum StatConfidenceLevels {
    STAT_CONFIDENCE_LEVEL_90 = 0,
    STAT_CONFIDENCE_LEVEL_95 = 1,
    STAT_CONFIDENCE_LEVEL_99 = 2
};

typedef struct stat_sample stat_sample_t;

stat_sample_t *stat_sample_create();
void stat_sample_free(stat_sample_t *sample);
void stat_sample_add(stat_sample_t *sample, double val);
void stat_sample_add_dataset(stat_sample_t *sample, double *dataset, int size);
void stat_sample_clean(stat_sample_t *sample);

double stat_sample_mean(stat_sample_t *sample);
double stat_sample_var(stat_sample_t *sample);
double stat_sample_stddev(stat_sample_t *sample);
double stat_sample_stderr(stat_sample_t *sample);
double stat_sample_stderr_rel(stat_sample_t *sample);
double stat_sample_min(stat_sample_t *sample);
double stat_sample_max(stat_sample_t *sample);
int stat_sample_min_index(stat_sample_t *sample);
int stat_sample_max_index(stat_sample_t *sample);
int stat_sample_size(stat_sample_t *sample);

double stat_mean(double *data, int size);
double stat_var(double *data, int size);
double stat_stddev(double *data, int size);
double stat_stderr(double *data, int size);
double stat_stderr_rel(double *data, int size);
double stat_min(double *data, int size);
int stat_min_index(double *data, int size);
double stat_max(double *data, int size);
int stat_max_index(double *data, int size);

double stat_fmax2(double a, double b);
double stat_fmin2(double a, double b);

int stat_dataset_remove_outliers(double *data, int size, int lb, int ub);
int stat_sample_confidence_interval(stat_sample_t *sample, int level,
                                    double *lb, double *ub, double *err);
#endif /* STAT_H */
