#
# mpiperf makefile.
#

include ../Makefile.inc

mpiperf := mpiperf
mpiperf_objs := mpiperf.o seq.o util.o mempool.o bench.o stats.o \
                report.o timeslot.o mpigclock.o hpctimer.o logger.o \
                bench_coll.o bench_nbc.o bench_pt2pt.o \
                benchmarks/barrier.o \
                benchmarks/bcast.o \
                benchmarks/gather.o \
                benchmarks/scatter.o \
                benchmarks/allgather.o \
                benchmarks/alltoall.o \
                benchmarks/reduce.o \
                benchmarks/allreduce.o \
                benchmarks/reduce_scatter.o \
                benchmarks/scan.o \
                benchmarks/exscan.o \
                benchmarks/gatherv.o \
                benchmarks/scatterv.o \
                benchmarks/alltoallv.o \
                benchmarks/allgatherv.o \
                benchmarks/alltoallw.o \
                benchmarks/sendrecv.o \
                benchmarks/waitpattern.o \
                benchmarks/send.o
#               benchmarks/reduce_scatter_block.o

.PHONY: all clean

all: $(mpiperf)

$(mpiperf): $(mpiperf_objs)
	$(MPICC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(MPICC) $(CFLAGS) -c $< -o $@

benchmarks/%.o: /benchmarks/%.c
	$(MPICC) $(CFLAGS) -c $< -o $@

mpiperf.o: mpiperf.c mpiperf.h version.h
seq.o: seq.c seq.h
util.o: util.c util.h
bench.o: bench.c bench.h
mempool.o: mempool.c mempool.h
stats.o: stats.c stats.h
report.o: report.c report.h
timeslot.o: timeslot.c timeslot.h
mpigclock.o: mpigclock.c mpigclock.h
hpctimer.o: hpctimer.c hpctimer.h
logger.o: logger.c logger.h
bench_coll.o: bench_coll.c bench.h
bench_nbc.o: bench_nbc.c bench.h
bench_pt2pt.o: bench_pt2pt.c bench.h

benchmarks/barrier.h: benchmarks/barrier.c benchmarks/barrier.h
benchmarks/bcast.o: benchmarks/bcast.c benchmarks/bcast.h
benchmarks/gather.o: benchmarks/gather.c benchmarks/gather.h
benchmarks/gatherv.o: benchmarks/gatherv.c benchmarks/gatherv.h
benchmarks/scatter.o: benchmarks/scatter.c benchmarks/scatter.h
benchmarks/scatterv.o: benchmarks/scatterv.c benchmarks/scatterv.h
benchmarks/allgather.o: benchmarks/allgather.c benchmarks/allgather.h
benchmarks/allgatherv.o: benchmarks/allgatherv.c benchmarks/allgatherv.h
benchmarks/alltoall.o: benchmarks/alltoall.c benchmarks/alltoall.h
benchmarks/alltoallv.o: benchmarks/alltoallv.c benchmarks/alltoallv.h
benchmarks/alltoallw.o: benchmarks/alltoallw.c benchmarks/alltoallw.h
benchmarks/reduce.o: benchmarks/reduce.c benchmarks/reduce.h
benchmarks/allreduce.o: benchmarks/allreduce.c benchmarks/allreduce.h
benchmarks/reduce_scatter_block.o: benchmarks/reduce_scatter_block.c benchmarks/reduce_scatter_block.h
benchmarks/reduce_scatter.o: benchmarks/reduce_scatter.c benchmarks/reduce_scatter.h
benchmarks/scan.o: benchmarks/scan.c benchmarks/scan.h
benchmarks/exscan.o: benchmarks/exscan.c benchmarks/exscan.h
benchmarks/waitpattern.o: benchmarks/waitpattern.c benchmarks/waitpattern.h
benchmarks/send.o: benchmarks/send.c benchmarks/send.h
benchmarks/sendrecv.o: benchmarks/sendrecv.c benchmarks/sendrecv.h

clean:
	@rm -rf *.o benchmarks/*.o $(mpiperf) 
