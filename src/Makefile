#
# mpiperf makefile.
#

include ../Makefile.inc

mpiperf := mpiperf
mpiperf_objs := mpiperf.o util.o mempool.o stat.o report.o timeslot.o \
                mpigclock.o hpctimer.o logger.o \
                bench_coll.o \
                coll/allgather.o \
                coll/allgatherv.o \
                coll/allreduce.o \
                coll/alltoall.o \
                coll/alltoallv.o \
                coll/alltoallw.o \
                coll/barrier.o \
                coll/bcast.o \
                coll/exscan.o \
                coll/gather.o \
                coll/gatherv.o \
                coll/reduce_scatter_block.o \
                coll/reduce_scatter.o \
                coll/reduce.o \
                coll/scan.o \
                coll/scatter.o \
                coll/scatterv.o \
                coll/waitpattern.o \
                bench_pt2pt.o \
                pt2pt/send.o \
                pt2pt/sendrecv.o \
                bench_nbc.o \
                nbc/ialltoall.o \
                nbc/ialltoallv.o \
                nbc/iallgather.o \
                nbc/iallgatherv.o \
                nbc/iallreduce.o \
                nbc/ibarrier.o \
                nbc/ibcast.o

.PHONY: all clean

all: $(mpiperf)

$(mpiperf): $(mpiperf_objs)
	$(MPICC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(MPICC) $(CFLAGS) -c $< -o $@

coll/%.o: /coll/%.c
	$(MPICC) $(CFLAGS) -c $< -o $@

nbc/%.o: /nbc/%.c
	$(MPICC) $(CFLAGS) -c $< -o $@

pt2pt/%.o: /pt2pt/%.c
	$(MPICC) $(CFLAGS) -c $< -o $@

mpiperf.o: mpiperf.c mpiperf.h version.h
seq.o: seq.c seq.h
util.o: util.c util.h
bench_coll.o: bench_coll.c bench_coll.h bench_coll_tab.h
bench_pt2pt.o: bench_pt2pt.c bench_pt2pt.h bench_pt2pt_tab.h
bench_nbc.o: bench_nbc.c bench_nbc.h bench_nbc_tab.h
mempool.o: mempool.c mempool.h
stat.o: stat.c stat.h
report.o: report.c report.h
timeslot.o: timeslot.c timeslot.h
mpigclock.o: mpigclock.c mpigclock.h
hpctimer.o: hpctimer.c hpctimer.h
logger.o: logger.c logger.h

coll/barrier.h: coll/barrier.c coll/barrier.h
coll/bcast.o: coll/bcast.c coll/bcast.h
coll/gather.o: coll/gather.c coll/gather.h
coll/gatherv.o: coll/gatherv.c coll/gatherv.h
coll/scatter.o: coll/scatter.c coll/scatter.h
coll/scatterv.o: coll/scatterv.c coll/scatterv.h
coll/allgather.o: coll/allgather.c coll/allgather.h
coll/allgatherv.o: coll/allgatherv.c coll/allgatherv.h
coll/alltoall.o: coll/alltoall.c coll/alltoall.h
coll/alltoallv.o: coll/alltoallv.c coll/alltoallv.h
coll/alltoallw.o: coll/alltoallw.c coll/alltoallw.h
coll/reduce.o: coll/reduce.c coll/reduce.h
coll/allreduce.o: coll/allreduce.c coll/allreduce.h
coll/reduce_scatter_block.o: coll/reduce_scatter_block.c coll/reduce_scatter_block.h
coll/reduce_scatter.o: coll/reduce_scatter.c coll/reduce_scatter.h
coll/scan.o: coll/scan.c coll/scan.h
coll/exscan.o: coll/exscan.c coll/exscan.h
coll/waitpattern.o: coll/waitpattern.c coll/waitpattern.h
pt2pt/send.o: pt2pt/send.c pt2pt/send.h
pt2pt/sendrecv.o: pt2pt/sendrecv.c pt2pt/sendrecv.h
nbc/ialltoall.o: nbc/ialltoall.c nbc/ialltoall.h
nbc/ialltoallv.o: nbc/ialltoallv.c nbc/ialltoallv.h
nbc/iallgather.o: nbc/iallgather.c nbc/iallgather.h
nbc/iallgatherv.o: nbc/iallgatherv.c nbc/iallgatherv.h
nbc/iallreduce.o: nbc/iallreduce.c nbc/iallreduce.h
nbc/ibarrier.o: nbc/ibarrier.c nbc/ibarrier.h
nbc/ibcast.o: nbc/ibcast.c nbc/ibcast.h

clean:
	@rm -rf *.o coll/*.o nbc/*.o pt2pt/*.o $(mpiperf) 
