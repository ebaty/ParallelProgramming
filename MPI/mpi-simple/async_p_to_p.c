#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define kSize 1LL<<20

int rank;
int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	const int kPrefix = 10000000;
	int sendData[kSize];
	int i;
	for(i = 0; i < kSize; ++i) sendData[i] = kPrefix * ((rank+1)&1) + i;

	MPI_Status waitStatus;

	if ( !(rank & 1) ) {
		printf("start Isend: %d -> %d\n", rank, (rank+1)&1);
		MPI_Request sendReq;
		MPI_Isend(sendData, kSize, MPI_INT, ((rank+1)&1), 0, MPI_COMM_WORLD, &sendReq);

		MPI_Wait(&sendReq, &waitStatus);
		printf("end Isend: %d\n", rank);
	}


	if ( !((rank+1) & 1) ) {
		int recvData[kSize];
		printf("start Irecv: %d <- %d\n", rank, (rank+1)&1);
		MPI_Request recvReq;
		MPI_Irecv(recvData, kSize, MPI_INT, ((rank+1)&1), 0, MPI_COMM_WORLD, &recvReq);

		MPI_Wait(&recvReq, &waitStatus);
		printf("end Irecv: %d\n", rank);

		int dataValidation = 0;
		for(i = 0; i < kSize; ++i) if ( recvData[i] != kPrefix * (rank&1) + i) dataValidation = 1;
		printf("rank = %d, data validation = %d [0 correct]\n", rank, dataValidation);
	}

	MPI_Finalize();

	return 0;
}
