#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
	int rank, tag = 0;
	char sendbuf[1024];
	char recvbuf[1024];

	MPI_Status status;
	MPI_Request hoge;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("rank = %d\n", rank);
	
	if ((rank & 1) == 0) {
		sprintf(sendbuf, "hoge!!! from %d", rank);
		int i, error;

		MPI_Isend(sendbuf, 1024, MPI_BYTE, rank+1, tag, MPI_COMM_WORLD, &hoge);
	} 
	else {
		MPI_Recv(recvbuf, 1024, MPI_BYTE, rank-1, tag, MPI_COMM_WORLD, &status);
		printf("recv string = %s\n", recvbuf);
	}

	MPI_Finalize();
	return 0;
}

