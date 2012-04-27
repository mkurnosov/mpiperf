/*
 * bench_coll_tab.h:
 *
 * Copyright (C) 2011-2012 Mikhail Kurnosov
 */

#ifndef BENCH_COLL_TAB_H
#define BENCH_COLL_TAB_H

#include "coll/allgather.h"
#include "coll/allgatherv.h"
#include "coll/allreduce.h"
#include "coll/alltoall.h"
#include "coll/alltoallv.h"
#include "coll/alltoallw.h"
#include "coll/barrier.h"
#include "coll/bcast.h"
#include "coll/exscan.h"
#include "coll/gather.h"
#include "coll/gatherv.h"
#include "coll/reduce_scatter_block.h"
#include "coll/reduce_scatter.h"
#include "coll/reduce.h"
#include "coll/scan.h"
#include "coll/scatter.h"
#include "coll/scatterv.h"
#include "coll/waitpattern.h"

collbench_t collbenchtab[] = {
    {
      "Allgather",
      (collbench_init_ptr_t)bench_allgather_init,
      (collbench_free_ptr_t)bench_allgather_free,
      (collbench_printinfo_ptr_t)bench_allgather_printinfo,
      (collbench_op_ptr_t)measure_allgather_sync
    },
    {
      "Allgatherv",
      (collbench_init_ptr_t)bench_allgatherv_init,
      (collbench_free_ptr_t)bench_allgatherv_free,
      (collbench_printinfo_ptr_t)bench_allgatherv_printinfo,
      (collbench_op_ptr_t)measure_allgatherv_sync
    },
    {
      "Allreduce",
      (collbench_init_ptr_t)bench_allreduce_init,
      (collbench_free_ptr_t)bench_allreduce_free,
      (collbench_printinfo_ptr_t)bench_allreduce_printinfo,
      (collbench_op_ptr_t)measure_allreduce_sync
    },
    {
      "Alltoall",
      (collbench_init_ptr_t)bench_alltoall_init,
      (collbench_free_ptr_t)bench_alltoall_free,
      (collbench_printinfo_ptr_t)bench_alltoall_printinfo,
      (collbench_op_ptr_t)measure_alltoall_sync
    },
    {
      "Alltoallv",
      (collbench_init_ptr_t)bench_alltoallv_init,
      (collbench_free_ptr_t)bench_alltoallv_free,
      (collbench_printinfo_ptr_t)bench_alltoallv_printinfo,
      (collbench_op_ptr_t)measure_alltoallv_sync
    },
    {
      "Alltoallw",
      (collbench_init_ptr_t)bench_alltoallw_init,
      (collbench_free_ptr_t)bench_alltoallw_free,
      (collbench_printinfo_ptr_t)bench_alltoallw_printinfo,
      (collbench_op_ptr_t)measure_alltoallw_sync
    },
    {
      "Barrier",
      (collbench_init_ptr_t)bench_barrier_init,
      (collbench_free_ptr_t)bench_barrier_free,
      (collbench_printinfo_ptr_t)bench_barrier_printinfo,
      (collbench_op_ptr_t)measure_barrier_sync
    },
    {
      "Bcast",
      (collbench_init_ptr_t)bench_bcast_init,
      (collbench_free_ptr_t)bench_bcast_free,
      (collbench_printinfo_ptr_t)bench_bcast_printinfo,
      (collbench_op_ptr_t)measure_bcast_sync
    },
    {
      "Exscan",
      (collbench_init_ptr_t)bench_exscan_init,
      (collbench_free_ptr_t)bench_exscan_free,
      (collbench_printinfo_ptr_t)bench_exscan_printinfo,
      (collbench_op_ptr_t)measure_exscan_sync
    },
    {
      "Gather",
      (collbench_init_ptr_t)bench_gather_init,
      (collbench_free_ptr_t)bench_gather_free,
      (collbench_printinfo_ptr_t)bench_gather_printinfo,
      (collbench_op_ptr_t)measure_gather_sync
    },
    {
      "Gatherv",
      (collbench_init_ptr_t)bench_gatherv_init,
      (collbench_free_ptr_t)bench_gatherv_free,
      (collbench_printinfo_ptr_t)bench_gatherv_printinfo,
      (collbench_op_ptr_t)measure_gatherv_sync
    },
#ifdef MPICH2
    {
      "Reduce_scatter_block",
      (collbench_init_ptr_t)bench_reduce_scatter_block_init,
      (collbench_free_ptr_t)bench_reduce_scatter_block_free,
      (collbench_printinfo_ptr_t)bench_reduce_scatter_block_printinfo,
      (collbench_op_ptr_t)measure_reduce_scatter_block_sync
    },
#endif
    {
      "Reduce_scatter",
      (collbench_init_ptr_t)bench_reduce_scatter_init,
      (collbench_free_ptr_t)bench_reduce_scatter_free,
      (collbench_printinfo_ptr_t)bench_reduce_scatter_printinfo,
      (collbench_op_ptr_t)measure_reduce_scatter_sync
    },
    {
      "Reduce",
      (collbench_init_ptr_t)bench_reduce_init,
      (collbench_free_ptr_t)bench_reduce_free,
      (collbench_printinfo_ptr_t)bench_reduce_printinfo,
      (collbench_op_ptr_t)measure_reduce_sync
    },
    {
      "Scan",
      (collbench_init_ptr_t)bench_scan_init,
      (collbench_free_ptr_t)bench_scan_free,
      (collbench_printinfo_ptr_t)bench_scan_printinfo,
      (collbench_op_ptr_t)measure_scan_sync
    },
    {
      "Scatter",
      (collbench_init_ptr_t)bench_scatter_init,
      (collbench_free_ptr_t)bench_scatter_free,
      (collbench_printinfo_ptr_t)bench_scatter_printinfo,
      (collbench_op_ptr_t)measure_scatter_sync
    },
    {
      "Scatterv",
      (collbench_init_ptr_t)bench_scatterv_init,
      (collbench_free_ptr_t)bench_scatterv_free,
      (collbench_printinfo_ptr_t)bench_scatterv_printinfo,
      (collbench_op_ptr_t)measure_scatterv_sync
    },
    {
      "WaitPatternUp",
      (collbench_init_ptr_t)bench_waitpattern_init,
      (collbench_free_ptr_t)bench_waitpattern_free,
      (collbench_printinfo_ptr_t)bench_waitpatternup_printinfo,
      (collbench_op_ptr_t)measure_waitpatternup_sync
    },
    {
      "WaitPatternDown",
      (collbench_init_ptr_t)bench_waitpattern_init,
      (collbench_free_ptr_t)bench_waitpattern_free,
      (collbench_printinfo_ptr_t)bench_waitpatterndown_printinfo,
      (collbench_op_ptr_t)measure_waitpatterndown_sync
    },
    {
      "WaitPatternNull",
      (collbench_init_ptr_t)bench_waitpattern_init,
      (collbench_free_ptr_t)bench_waitpattern_free,
      (collbench_printinfo_ptr_t)bench_waitpatternnull_printinfo,
      (collbench_op_ptr_t)measure_waitpatternnull_sync
    },
};

#endif /* BENCH_COLL_TAB_H */

