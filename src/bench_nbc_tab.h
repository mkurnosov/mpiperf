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
#include "nbc/ibarrier.h"
#include "nbc/ibcast.h"
#include "nbc/iexscan.h"
#include "nbc/igather.h"
#include "nbc/igatherv.h"
#include "nbc/ireduce_scatter_block.h"
#include "nbc/ireduce_scatter.h"
#include "nbc/ireduce.h"
#include "nbc/iscan.h"
#include "nbc/iscatter.h"

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
+igather
+igatherv
+ireduce_scatter_block
+ireduce_scatter
+ ireduce
+ iscan
+ iscatter
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
	"Igather",
    (nbcbench_init_ptr_t)bench_igather_init,
    (nbcbench_free_ptr_t)bench_igather_free,
    (nbcbench_printinfo_ptr_t)bench_igather_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_igather_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_igather_overlap,
},
{
	"Igatherv",
    (nbcbench_init_ptr_t)bench_igatherv_init,
    (nbcbench_free_ptr_t)bench_igatherv_free,
    (nbcbench_printinfo_ptr_t)bench_igatherv_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_igatherv_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_igatherv_overlap,
},
{
	"Ireduce_scatter_block",
    (nbcbench_init_ptr_t)bench_ireduce_scatter_block_init,
    (nbcbench_free_ptr_t)bench_ireduce_scatter_block_free,
    (nbcbench_printinfo_ptr_t)bench_ireduce_scatter_block_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ireduce_scatter_block_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ireduce_scatter_block_overlap,
},
{
	"Ireduce_scatter",
    (nbcbench_init_ptr_t)bench_ireduce_scatter_init,
    (nbcbench_free_ptr_t)bench_ireduce_scatter_free,
    (nbcbench_printinfo_ptr_t)bench_ireduce_scatter_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ireduce_scatter_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ireduce_scatter_overlap,
},
{
	"Ireduce",
    (nbcbench_init_ptr_t)bench_ireduce_init,
    (nbcbench_free_ptr_t)bench_ireduce_free,
    (nbcbench_printinfo_ptr_t)bench_ireduce_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_ireduce_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_ireduce_overlap,
},
{
	"Iscan",
    (nbcbench_init_ptr_t)bench_iscan_init,
    (nbcbench_free_ptr_t)bench_iscan_free,
    (nbcbench_printinfo_ptr_t)bench_iscan_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iscan_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iscan_overlap,
},
{
	"Iscatter",
    (nbcbench_init_ptr_t)bench_iscatter_init,
    (nbcbench_free_ptr_t)bench_iscatter_free,
    (nbcbench_printinfo_ptr_t)bench_iscatter_printinfo,
    (nbcbench_collop_blocking_ptr_t)measure_iscatter_blocking,
    (nbcbench_collop_overlap_ptr_t)measure_iscatter_overlap,
},
};
#endif
#endif /* BENCH_NBC_TAB_H */

