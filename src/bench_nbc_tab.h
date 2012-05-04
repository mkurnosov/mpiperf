/*
 * bench_nbc_tab.h:
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#ifndef BENCH_NBC_TAB_H
#define BENCH_NBC_TAB_H

#include "nbc/iallgather.h"
#include "nbc/iallgatherv.h"
#include "nbc/iallreduce.h"
#include "nbc/ialltoall.h"
#include "nbc/ialltoallv.h"
#include "nbc/ialltoallw.h"
#include "nbc/iexscan.h"
#include "nbc/ibarrier.h"
#include "nbc/ibcast.h"

/*
MPICH2 1.5b1 NBC routines:
+iallgather
+iallgatherv
+iallreduce
+ialltoall
+ialltoallv
+ialltoallw
+ibarrier
+ibcast
+iexscan
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
	"Iallgather",
    (nbcbench_init_ptr_t)bench_iallgather_init,
    (nbcbench_free_ptr_t)bench_iallgather_free,
    (nbcbench_printinfo_ptr_t)bench_iallgather_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iallgather_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iallgather_overlap,
},
{
	"Iallgatherv",
    (nbcbench_init_ptr_t)bench_iallgatherv_init,
    (nbcbench_free_ptr_t)bench_iallgatherv_free,
    (nbcbench_printinfo_ptr_t)bench_iallgatherv_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iallgatherv_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iallgatherv_overlap,
},
{
	"Iallreduce",
    (nbcbench_init_ptr_t)bench_iallreduce_init,
    (nbcbench_free_ptr_t)bench_iallreduce_free,
    (nbcbench_printinfo_ptr_t)bench_iallreduce_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iallreduce_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iallreduce_overlap,
},
{
	"Ialltoall",
    (nbcbench_init_ptr_t)bench_ialltoall_init,
    (nbcbench_free_ptr_t)bench_ialltoall_free,
    (nbcbench_printinfo_ptr_t)bench_ialltoall_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ialltoall_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ialltoall_overlap,
},
{
	"Ialltoallv",
    (nbcbench_init_ptr_t)bench_ialltoallv_init,
    (nbcbench_free_ptr_t)bench_ialltoallv_free,
    (nbcbench_printinfo_ptr_t)bench_ialltoallv_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ialltoallv_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ialltoallv_overlap,
},
{
	"Ialltoallw",
    (nbcbench_init_ptr_t)bench_ialltoallw_init,
    (nbcbench_free_ptr_t)bench_ialltoallw_free,
    (nbcbench_printinfo_ptr_t)bench_ialltoallw_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ialltoallw_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ialltoallw_overlap,
},
{
	"Iexscan",
    (nbcbench_init_ptr_t)bench_iexscan_init,
    (nbcbench_free_ptr_t)bench_iexscan_free,
    (nbcbench_printinfo_ptr_t)bench_iexscan_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iexscan_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iexscan_overlap,
},
{
	"Ibarrier",
    (nbcbench_init_ptr_t)bench_ibarrier_init,
    (nbcbench_free_ptr_t)bench_ibarrier_free,
    (nbcbench_printinfo_ptr_t)bench_ibarrier_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ibarrier_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ibarrier_overlap,
},
{
	"Ibcast",
    (nbcbench_init_ptr_t)bench_ibcast_init,
    (nbcbench_free_ptr_t)bench_ibcast_free,
    (nbcbench_printinfo_ptr_t)bench_ibcast_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ibcast_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ibcast_overlap,
}
#endif
};

#endif /* BENCH_NBC_TAB_H */

