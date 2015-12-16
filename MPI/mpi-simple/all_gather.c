#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define kMPIDataType MPI_LONG_LONG 
#define kDataType long long

#define kFileName "allgather.csv"
typedef struct mpi_result {
	int rank;
	long long size;
	double time;
}mpi_result;

mpi_result sendData(long long size, int node_size) {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	kDataType *sendbuf;
	kDataType *recvbuf;

	MPI_Status status;

	sendbuf = (kDataType *)malloc(size * sizeof(kDataType));
	recvbuf = (kDataType *)malloc(size * node_size * sizeof(kDataType));
	if ( sendbuf == NULL || recvbuf == NULL ) {
		printf("memory limit exceeed\n");
		mpi_result r;
		return r;
	}

	double start, end;

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();

	MPI_Allgather(sendbuf, size, kMPIDataType, recvbuf, size, kMPIDataType, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	end = MPI_Wtime();


	free(sendbuf);
	free(recvbuf);

	mpi_result result;
	result.rank = rank;
	result.size = size * sizeof(kDataType);
	result.time = end - start;

	return result;
}

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int node_size;
	MPI_Comm_size(MPI_COMM_WORLD, &node_size);

	// ファイルポインタの準備
	FILE *fp;
	fp = fopen(kFileName, "w");
	if ( rank == 0 ) {
		if ( fp == NULL ) {
			printf("can't open %s.\n", kFileName);
			return 1;
		}
	}

	long long i;
	for(i = 4; i <= (1LL << 27); i <<= 1) {
		long long size;
		double ave_time = 0;

		printf("%s, %d\n", __FUNCTION__, i);
		int j;
		for(j = 0; j < 10; ++j) {
			mpi_result result = sendData(i, node_size);
			ave_time += result.time;
			size = result.size;
		}
		ave_time /= 10.0f;

		if ( rank == 0 ) {
			fprintf(fp, "%lld\t%lf\n", size, ave_time);
		}
	}

	if ( rank == 0 ) fclose(fp);

	MPI_Finalize();

	return 0;
}
