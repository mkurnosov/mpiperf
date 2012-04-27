/*
 * usermpi.c:
 */ 

#include <mpi.h>

/*
 * This flag indicates that mpiperf finished all preparations and started
 * measurements. So we can call our version of function insted of native MPI
 * routine.
 */
extern int mpiperf_is_measure_started;

/* MPI_Bcast_user: This is our version of MPI function. */
int MPI_Bcast_user(void *buf, int count, MPI_Datatype datatype, int root,
                   MPI_Comm comm)
{
	int i, rank, commsize;

	PMPI_Comm_size(comm, &commsize);
	PMPI_Comm_rank(comm, &rank);

	/* Simple Linear algorithm for broadcast */
	if (rank == root) {
		for (i = 0; i < commsize; i++) {
			if (i != root)
				PMPI_Send(buf, count, datatype, i, 4321, comm);
		}
	} else {
		PMPI_Recv(buf, count, datatype, root, 4321, comm, MPI_STATUS_IGNORE);
	}
	return MPI_SUCCESS;
}

/* MPI_Bcast: This function is called by MPIPerf. */
int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
		      MPI_Comm comm)
{
	if (mpiperf_is_measure_started) {
	    /* MPIPerf started measurements, so we can call our function. */
		return MPI_Bcast_user(buf, count, datatype, root, comm);
	}
	return PMPI_Bcast(buf, count, datatype, root, comm);
}
