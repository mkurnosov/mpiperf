/*
 * bench_pt2pt_tab.h:
 *
 * Copyright (C) 2012 Mikhail Kurnosov
 */

#ifndef BENCH_PT2PT_TAB_H
#define BENCH_PT2PT_TAB_H

#include "pt2pt/send.h"
#include "pt2pt/sendrecv.h"

pt2ptbench_t pt2ptbenchtab[] = {
    {
      "Send",
      (pt2ptbench_init_ptr_t)bench_send_init,
      (pt2ptbench_free_ptr_t)bench_send_free,
      (pt2ptbench_printinfo_ptr_t)bench_send_printinfo,
      (pt2ptbench_op_ptr_t)measure_send_sync
    },
    {
      "Sendrecv",
      (pt2ptbench_init_ptr_t)bench_sendrecv_init,
      (pt2ptbench_free_ptr_t)bench_sendrecv_free,
      (pt2ptbench_printinfo_ptr_t)bench_sendrecv_printinfo,
      (pt2ptbench_op_ptr_t)measure_sendrecv_sync
    }
};

#endif /* BENCH_PT2PT_TAB_H */


