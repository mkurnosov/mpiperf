/*
 * seq.h: Sequence of parameter values.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef SEQ_H
#define SEQ_H

#include "bench.h"

enum {
    SEQ_STEP_INC = 0,       /* val[i] = val[i-1] + step */
    SEQ_STEP_MUL = 1        /* val[i] = val[i-1] * step */
};

int intseq_initialize(bench_t *bench, int min, int max, int step, int step_type);
int intseq_finalize(bench_t *bench);
int intseq_getcurrent(bench_t *bench);
int intseq_getnext(bench_t *bench);
int intseq_getsize(bench_t *bench);
void intseq_reset(bench_t *bench);

int commseq_initialize(bench_t *bench, int min, int max, int step, int step_type);
int commseq_finalize(bench_t *bench);
int commseq_getcurrent(bench_t *bench);
int commseq_getnext(bench_t *bench);
int commseq_getsize(bench_t *bench);
void commseq_reset(bench_t *bench);
MPI_Comm commseq_getcomm(bench_t *bench);
int commseq_getcommsize(bench_t *bench);
int commseq_getcommrank(bench_t *bench);

#endif /* SEQ_H */
