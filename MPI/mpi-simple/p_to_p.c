#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define kMPIDataType MPI_LONG_LONG 
#define kDataType long long

#define kFileName "points_to_points.csv"
typedef struct mpi_result {
	int rank;
	long long size;
	double time;
}mpi_result;

mpi_result sendData(int size) {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	kDataType *sendbuf;
	kDataType *recvbuf;

	MPI_Status status;

	sendbuf = (kDataType *)malloc(size * sizeof(kDataType));
	recvbuf = (kDataType *)malloc(size * sizeof(kDataType));
	if ( sendbuf == NULL || recvbuf == NULL ) {
		printf("memory limit exceeed\n");
		mpi_result r;
		return r;
	}

	double start, end;

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();

	if ( (rank & 1) == 0 ) {
		sendbuf[size-1] = size;
		printf("send from: %d, size: %d\n", rank, size);
		int error = MPI_Send(sendbuf, size, kMPIDataType, rank+1, 0, MPI_COMM_WORLD);
	}else {
		int error = MPI_Recv(recvbuf, size, kMPIDataType, rank-1, 0, MPI_COMM_WORLD, &status);
		error += (recvbuf[size-1] == size) ? 0 : 1;
		printf("received:  %d, size: %d, error: %s\n", rank, size, error ? "true" : "false");
	}

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

	// ファイルポインタの準備
	FILE *fp = fopen(kFileName, "w");
	if ( fp == NULL ) {
		printf("can't open %s.\n", kFileName);
		return 1;
	}

	long long i;
	for(i = 1; i <= (1LL << 31); i <<= 1) {
		long long size;
		double ave_time = 0;
		if ( i == 2147483648 ) i = 2147483647;

		printf("%s, %d\n", __PRETTY_FUNCTION__, i);
		int j;
		for(j = 0; j < 10; ++j) {
			mpi_result result = sendData(i);
			ave_time += result.time;
			size = result.size;
		}
		ave_time /= 10.0f;

		fprintf(fp, "%lld\t%lf\n", size, ave_time);
	}

	fclose(fp);

	MPI_Finalize();

	return 0;
}
