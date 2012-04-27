/*
 * bench_nbc_tab.h:
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#ifndef BENCH_NBC_TAB_H
#define BENCH_NBC_TAB_H

#include "nbc/iallreduce.h"
#include "nbc/ibarrier.h"

/*
MPICH2 1.5b1 NBC routines:
iallgather
iallgatherv
iallreduce
ialltoall
ialltoallv
ialltoallw
ibarrier
ibcast
iexscan
igather
igatherv
ired_scat
ired_scat_block
ireduce
iscan
iscatter
iscatterv
*/

nbcbench_t nbcbenchtab[] = {
#if MPICH2_NUMVERSION >= 10500002
{
	"Iallreduce",
    (nbcbench_init_ptr_t)bench_iallreduce_init,
    (nbcbench_free_ptr_t)bench_iallreduce_free,
    (nbcbench_printinfo_ptr_t)bench_iallreduce_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iallreduce_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iallreduce_overlap,
},
{
	"Ibarrier",
    (nbcbench_init_ptr_t)bench_ibarrier_init,
    (nbcbench_free_ptr_t)bench_ibarrier_free,
    (nbcbench_printinfo_ptr_t)bench_ibarrier_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ibarrier_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ibarrier_overlap,
},
#endif
};

#endif /* BENCH_NBC_TAB_H */

