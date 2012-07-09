#!/bin/sh

#mpiexec -np 2 ./mpiperf -m -l log -r10 -R100 -e3 allgather $@
#mpiexec -np 2 ./mpiperf -x1 -X65536 -S2 -e3 -r10 -R100 allgather $@
#mpiexec -np 4 ./mpiperf -t tsc -p -l log -x4 -X512 -S2 -e3 -r10 -R100 sendrecv 
#mpiexec -np 4 ./mpiperf -l log -x4 -X512 -S2 -e3 -r10 -R100 allgather $@

#mpiexec -np 4 ./mpiperf -l log -x1 -X524288 -S2 -e3 -r8 -R32 allgather
#mpiexec -np 4 ./mpiperf -x1024 -X65535 -r10 -R100 -e3 -s1 Bcast

#mpiexec -np 2 ./mpiperf -w usec -t tsc -x1000 -X2000 -S2 -r10 -R100 -e3 Bcast
#mpiexec -np 2 ./mpiperf -x1000 -X2000 -S2 -r10 -R100 -e3 Bcast

#mpiexec -np 2 ./mpiperf -w sec -p -llog -t tsc -z nosync -x1 -X1MiB -S2 -r10 -R30 -e3 Bcast

#mpiexec -np 2 ./mpiperf -b Allgather -l 1 -u 4096 -r 30
#mpiexec -np 2 ./mpiperf -b Allgather_msgsize -l 1 -u 4096 -r 30
#mpiexec -np 2 ./mpiperf -b Allgather_commsize -l 1 -u 4096 -r 100
#mpiexec -np 2 ./mpiperf -q -- list of benchmarks

#mpiexec -np 2 ./mpiperf -m -l log -o out -x64 -X1024 iallreduce
#mpiexec -np 3 ./mpiperf -p3 -P3 -x100 -X100 -b -o report ireduce








