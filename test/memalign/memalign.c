#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

int buf_alignment = 32;

static int alignment_fix(char *p, int a)
{
    MPI_Aint absolute_address;
    unsigned long address;
    int fix;
      
    MPI_Get_address(p, &absolute_address);
    address = (unsigned long) absolute_address;
    fix = (a - (address % a)) % a;
    
    if ( (address + fix) % (2 * a) == 0) {
        fix += a;
    }
    return fix;                                                
}
                                                                                            
static char *align_ptr(char *p, int a)
{
    return p + alignment_fix(p, a);
}
  
void *alloc(int size)
{
    char *buf;
    
    /*
    size = size + 8 * buf_alignment;
    buf = malloc(size);
    buf = align_ptr(buf, buf_alignment);
    memset(buf, 0x33, size);
    */
    buf = malloc(size);
    return buf;
}

int main(int argc, char **argv)
{
    int rank, commsize, i;
    char *sbuf, *rbuf;
    int scount, rcount;
    double t, ttotal = 0.0;
    int n = 8;
                            
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    scount = 1024;
    rcount = 1024 * commsize;
    sbuf = alloc(scount);
    rbuf = alloc(rcount);
    
    t = MPI_Wtime();
    MPI_Allgather(sbuf, scount, MPI_BYTE, rbuf, rcount, MPI_BYTE, MPI_COMM_WORLD);
    t = MPI_Wtime() - t;

    MPI_Barrier(MPI_COMM_WORLD);

    for (i = 0; i < n; i++) {
        t = MPI_Wtime();
        MPI_Allgather(sbuf, scount, MPI_BYTE, rbuf, scount, MPI_BYTE, MPI_COMM_WORLD);
        t = MPI_Wtime() - t;
        ttotal += t;
    }

    printf("Process %d time: %.9f  %.1f\n", rank, ttotal / n, ttotal * 1e6 / n);
    
    MPI_Finalize();
    return 0;
}
