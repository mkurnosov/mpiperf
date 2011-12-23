/*
 * stats.c: Statistical functions.
 *
 * Copyright (C) 2010-2011 Mikhail Kurnosov
 */

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "stats.h"

/* Statistical sample */
struct stats_sample {
    double sum;         /* Sum of sample elements: x_0 + x_1 + ... + x_n */
    double sum_pow2;    /* Sum of elements squares: {x_0}^2 + {x_1}^2 ... */
    double min;
    double max;
    int min_index;      /* Elements are numbered from 0: 0, 1, 2, ... */
    int max_index;
    int size;           /* Number of elements in sample */
};

/*
 * Student's t-distribution (two tails probability)
 */
static int tstud_sample_size = 100;
static double tstud_p90_ninf = 1.645;
static double tstud_tab_p90[] = {
    6.314, 2.920, 2.353, 2.132, 2.015, 1.943, 1.895,
    1.860, 1.833, 1.812, 1.796, 1.782, 1.771, 1.761, 1.753,
    1.746, 1.740, 1.734, 1.729, 1.725, 1.721, 1.717, 1.714,
    1.711, 1.708, 1.706, 1.703, 1.701, 1.699, 1.697, 1.696,
    1.694, 1.692, 1.691, 1.690, 1.688, 1.687, 1.686, 1.685,
    1.684, 1.683, 1.682, 1.681, 1.680, 1.679, 1.679, 1.678,
    1.677, 1.677, 1.676, 1.675, 1.675, 1.674, 1.674, 1.673,
    1.673, 1.672, 1.672, 1.671, 1.671, 1.670, 1.670, 1.669,
    1.669, 1.669, 1.668, 1.668, 1.668, 1.667, 1.667, 1.667,
    1.666, 1.666, 1.666, 1.665, 1.665, 1.665, 1.665, 1.664,
    1.664, 1.664, 1.664, 1.663, 1.663, 1.663, 1.663, 1.663,
    1.662, 1.662, 1.662, 1.662, 1.662, 1.661, 1.661, 1.661,
    1.661, 1.661, 1.661, 1.660
};

static double tstud_p95_ninf = 1.960;
static double tstud_tab_p95[] = {
    12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365,
    2.306, 2.262, 2.228, 2.201, 2.179, 2.160, 2.145, 2.131,
    2.120, 2.110, 2.101, 2.093, 2.086, 2.080, 2.074, 2.069,
    2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042, 2.040,
    2.037, 2.035, 2.032, 2.030, 2.028, 2.026, 2.024, 2.023,
    2.021, 2.020, 2.018, 2.017, 2.015, 2.014, 2.013, 2.012,
    2.011, 2.010, 2.009, 2.008, 2.007, 2.006, 2.005, 2.004,
    2.003, 2.002, 2.002, 2.001, 2.000, 2.000, 1.999, 1.998,
    1.998, 1.997, 1.997, 1.996, 1.995, 1.995, 1.994, 1.994,
    1.993, 1.993, 1.993, 1.992, 1.992, 1.991, 1.991, 1.990,
    1.990, 1.990, 1.989, 1.989, 1.989, 1.988, 1.988, 1.988,
    1.987, 1.987, 1.987, 1.986, 1.986, 1.986, 1.986, 1.985,
    1.985, 1.985, 1.984, 1.984
};

static double tstud_p99_ninf = 2.576;
static double tstud_tab_p99[] = {
    63.657, 9.925, 5.841, 4.604, 4.032, 3.707, 3.499,
    3.355, 3.250, 3.169, 3.106, 3.055, 3.012, 2.977, 2.947,
    2.921, 2.898, 2.878, 2.861, 2.845, 2.831, 2.819, 2.807,
    2.797, 2.787, 2.779, 2.771, 2.763, 2.756, 2.750, 2.744,
    2.738, 2.733, 2.728, 2.724, 2.719, 2.715, 2.712, 2.708,
    2.704, 2.701, 2.698, 2.695, 2.692, 2.690, 2.687, 2.685,
    2.682, 2.680, 2.678, 2.676, 2.674, 2.672, 2.670, 2.668,
    2.667, 2.665, 2.663, 2.662, 2.660, 2.659, 2.657, 2.656,
    2.655, 2.654, 2.652, 2.651, 2.650, 2.649, 2.648, 2.647,
    2.646, 2.645, 2.644, 2.643, 2.642, 2.641, 2.640, 2.640,
    2.639, 2.638, 2.637, 2.636, 2.636, 2.635, 2.634, 2.634,
    2.633, 2.632, 2.632, 2.631, 2.630, 2.630, 2.629, 2.629,
    2.628, 2.627, 2.627, 2.626
};

static int fcmp(const void *p, const void *q);

/* stats_sample_create: */
stats_sample_t *stats_sample_create()
{
    stats_sample_t *sample;

    if ( (sample = malloc(sizeof(*sample))) == NULL) {
        return NULL;
    }
    stats_sample_clean(sample);
    return sample;
}

/* stats_sample_free: */
void stats_sample_free(stats_sample_t *sample)
{
    free(sample);
}

/* stats_sample_add: */
void stats_sample_add(stats_sample_t *sample, double val)
{
    sample->sum += val;
    sample->sum_pow2 += val * val;

    if (val < sample->min) {
        sample->min = val;
        sample->min_index = sample->size;
    }
    if (val > sample->max) {
        sample->max = val;
        sample->max_index = sample->size;
    }
    sample->size++;
}

/* stats_sample_add_dataset: */
void stats_sample_add_dataset(stats_sample_t *sample, double *dataset, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        stats_sample_add(sample, dataset[i]);
    }
}


/* stats_sample_clean: */
void stats_sample_clean(stats_sample_t *sample)
{
    sample->size = 0;
    sample->sum = 0.0;
    sample->sum_pow2 = 0.0;
    sample->min = DBL_MAX;
    sample->max = 0.0;
    sample->min_index = -1;
    sample->max_index = -1;
}

/* stats_sample_mean: */
double stats_sample_mean(stats_sample_t *sample)
{
    if (sample->size > 1) {
        return sample->sum / (double)sample->size;
    } else {
        return sample->sum;
    }
}

/* stats_sample_var: */
double stats_sample_var(stats_sample_t *sample)
{
    if (sample->size > 1) {
        return 1.0 / (sample->size - 1.0) * sample->sum_pow2 -
               (1.0 / (sample->size * (sample->size - 1.0))) *
               sample->sum * sample->sum;
    } else {
        return 0.0;
    }
}

/* stats_sample_stddev: */
double stats_sample_stddev(stats_sample_t *sample)
{
    return sqrt(stats_sample_var(sample));
}

/* stats_sample_stderr: */
double stats_sample_stderr(stats_sample_t *sample)
{
    return stats_sample_stddev(sample) / sqrt(sample->size);
}

/* stats_sample_stderr_rel: */
double stats_sample_stderr_rel(stats_sample_t *sample)
{
    double stderr = stats_sample_stderr(sample);
    return (stderr > 0.0) ? stderr / stats_sample_mean(sample) : 0.0;
}

/* stats_sample_min: */
double stats_sample_min(stats_sample_t *sample)
{
    return sample->min;
}

/* stats_sample_max: */
double stats_sample_max(stats_sample_t *sample)
{
    return sample->max;
}

/* stats_sample_min_index: */
int stats_sample_min_index(stats_sample_t *sample)
{
    return sample->min_index;
}

/* stats_sample_max_index: */
int stats_sample_max_index(stats_sample_t *sample)
{
    return sample->max_index;
}

/* stats_sample_size: */
int stats_sample_size(stats_sample_t *sample)
{
    return sample->size;
}

/* stats_mean: Returns sample mean of the dataset. */
double stats_mean(double *data, int size)
{
    int i;
    double sum = 0.0;

    if (size > 1) {
        for (i = 0; i < size; i++)
            sum += data[i];
        return sum / (double)size;
    } else {
        return data[0];
    }
}

/*
 * stats_var: Returns sample variance of the dataset (unbiased estimator).
 *            \sigma^2 = (1/(N-1)) \sum (x_i - \Hat x)^2.
 */
double stats_var(double *data, int size)
{
    int i;
    double sum = 0.0, sum_pow2 = 0.0;

    if (size > 1) {
        for (i = 0; i < size; i++) {
            sum += data[i];
            sum_pow2 += data[i] * data[i];
        }
        return 1.0 / (size - 1.0) * sum_pow2 -
               (1.0 / (size * (size - 1.0))) * sum * sum;
    } else {
        return 0.0;
    }
}

/*
 * stats_stddev: Returns sample standard deviation of the dataset.
 *               s = \sqrt{(1/(N-1)) \sum (x_i - \Hat x)^2}
 */
double stats_stddev(double *data, int size)
{
    return sqrt(stats_var(data, size));
}

/*
 * stats_stderr: Returns sample standard error of the mean (SEM).
 *               stderr = s / \sqrt{N}
 */
double stats_stderr(double *data, int size)
{
    return stats_stddev(data, size) / sqrt(size);
}

/*
 * stats_stderr_rel: Returns sample relative standard error (RSE) of the dataset.
 *                   stderr = (s / \sqrt{N}) / \Hat x
 */
double stats_stderr_rel(double *data, int size)
{
    return stats_stderr(data, size) / stats_mean(data, size);
}

/* stats_min: */
double stats_min(double *data, int size)
{
    int i;
    double min;

    for (min = data[0], i = 1; i < size; i++) {
        if (min > data[i])
            min = data[i];
    }
    return min;
}

/* stats_min_index: */
int stats_min_index(double *data, int size)
{
    int i, imin;

    for (imin = 0, i = 1; i < size; i++) {
        if (data[imin] > data[i])
            imin = i;
    }
    return imin;
}

/* stats_max: */
double stats_max(double *data, int size)
{
    int i;
    double max;

    for (max = data[0], i = 1; i < size; i++) {
        if (max < data[i])
            max = data[i];
    }
    return max;
}

/* stat_max_index: */
int stats_max_index(double *data, int size)
{
    int i, imax;

    for (imax = 0, i = 1; i < size; i++) {
        if (data[imax] < data[i])
            imax = i;
    }
    return imax;
}

double stats_fmax2(double a, double b)
{
    return (a > b) ? a : b;
}

double stats_fmin2(double a, double b)
{
    return (a < b) ? a : b;
}

/* fcmp: Compares two elements of type double. */
static int fcmp(const void *a, const void *b)
{
    if (*(double *)a < *(double *)b) {
        return -1;
    } else if (*(double *)a > *(double *)b) {
        return 1;
    }
    return 0;
}

/*
 * stats_dataset_remove_extreme: Removes lb percents of minimal values from
 *                               dataset and ub percents of maximal values.
 *                               Returns size of modified dataset
 *                               and -1 on error.
 */
int stats_dataset_remove_extreme(double *data, int size, int lb, int ub)
{
    int i, nmin, nmax, newsize;

    if (!data || (lb + ub > 100)) {
        return -1;
    }
    if (size == 0 || (lb + ub == 100)) {
        return 0;
    }

    qsort(data, size, sizeof(*data), fcmp);

    nmin = size / 100.0 * lb;
    nmax = size / 100.0 * ub;
    newsize = size - nmin - nmax;
    for (i = 0; i < newsize; i++) {
        data[i] = data[i + nmin];
    }
    return newsize;
}

/*
 *  stats_sample_confidence_interval: Returns confidence interval for the given
 *                                    sample and given confidence level.
 */
int stats_sample_confidence_interval(stats_sample_t *sample, int level,
                                     double *lb, double *ub, double *err)
{
    double mean, t;

    if (!sample)
        return -1;

    if (sample->size == 0)
        return -1;

    if (sample->size == 1) {
        *err = 0.0;
        *lb = *ub = stats_sample_mean(sample);
    }

    if (level == STATS_CONFIDENCE_LEVEL_90) {
        t = (sample->size <= tstud_sample_size) ? tstud_tab_p90[sample->size - 2]
                                                : tstud_p90_ninf;
    } else if (level == STATS_CONFIDENCE_LEVEL_95) {
        t = (sample->size <= tstud_sample_size) ? tstud_tab_p95[sample->size - 2]
                                                : tstud_p95_ninf;
    } else if (level == STATS_CONFIDENCE_LEVEL_99) {
        t = (sample->size <= tstud_sample_size) ? tstud_tab_p99[sample->size - 2]
                                                : tstud_p99_ninf;
    } else {
        return -1;
    }
    *err = stats_sample_stderr(sample) * t;
    mean = stats_sample_mean(sample);
    *lb = mean - *err;
    *ub = mean + *err;
    return 0;
}
