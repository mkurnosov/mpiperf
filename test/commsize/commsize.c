/*
 * commsize.c:
 */
 
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

/*
 * getworldrank: Translate rank of process from communicator comm to
 *              communicator MPI_COMM_WORLD.
 */
int getworldrank(MPI_Comm comm, int rank)
{
    static MPI_Group worldgroup;
    static int isfirstcall = 1;
    int worldrank, isintercomm = 0;
    MPI_Group group;
                
    MPI_Comm_test_inter(comm, &isintercomm);
    if (isintercomm) {
        MPI_Comm_remote_group(comm, &group);
    } else {
        MPI_Comm_group(comm, &group);
    }
    
    if (isfirstcall) {
        MPI_Comm_group(MPI_COMM_WORLD, &worldgroup);
        isfirstcall = 0;
    }
    MPI_Group_translate_ranks(group, 1, &rank, worldgroup, &worldrank);
    MPI_Group_free(&group);
    return worldrank;
}

int main(int argc, char **argv)
{
    int rank, commsize, newsize, newrank;
    MPI_Comm newcomm;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    for (newsize = 1; newsize <= commsize; newsize++) {
        MPI_Comm_split(MPI_COMM_WORLD, (rank < newsize) ? 1 : MPI_UNDEFINED, commsize - rank - 1, &newcomm);
        if (newcomm == MPI_COMM_NULL) {
            continue;
        }
        MPI_Comm_rank(newcomm, &newrank);
        printf("%d | process %d now is %d (world = %d)\n", newsize, rank, newrank,
               getworldrank(newcomm, newrank));
        MPI_Comm_free(&newcomm);
    }
    
    MPI_Finalize();
    return 0;
}

