#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include "data_util_bin.c"
#include <stddef.h>

#include <omp.h>
#include <mpi.h>

#define STEP 10
#define DT 1.0f
#define G 1.0f
#define REP(i,n) for(i=0;i<n;++i)
int nodeSize;
int nodeRank;
int fileSize;
int arraySize;
char* fileHeader;
char* fileHooter;

MPI_Datatype mpi_matter_type;

typedef struct matter {
	double m, x, y, z, vx, vy, vz;
}matter;

void createDataType() {
	int i;
	int blockLength[7]; REP(i, 7) blockLength[i] = 1;
	MPI_Datatype types[7]; REP(i, 7) types[i] = MPI_DOUBLE;

	MPI_Aint offsets[7];
	offsets[0] = offsetof(matter, m);
	offsets[1] = offsetof(matter, x);
	offsets[2] = offsetof(matter, y);
	offsets[3] = offsetof(matter, z);
	offsets[4] = offsetof(matter, vx);
	offsets[5] = offsetof(matter, vy);
	offsets[6] = offsetof(matter, vz);

	MPI_Type_create_struct(7, blockLength, offsets, types, &mpi_matter_type);
	MPI_Type_commit(&mpi_matter_type);
}

void printThreads() {
	int count;
	#pragma omp parallel
	{
		count = omp_get_num_threads();
	}
	printf("Use threads: %2d\n", count);
}

char fileName[1024] = "";
char* getPath(char *s) {
	int i;
	REP(i, 1024) fileName[i] = '\0';

	strcat(fileName, fileHeader);
	strcat(fileName, s);
	strcat(fileName, fileHooter);

	return fileName;
}

void initMatter(matter *m) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * fileSize);

	int i;

	read_data(getPath("m"), buf, fileSize);
	REP(i, fileSize) m[i].m = buf[i];

	read_data(getPath("x"), buf, fileSize);
	REP(i, fileSize) m[i].x = buf[i];

	read_data(getPath("y"), buf, fileSize);
	REP(i, fileSize) m[i].y = buf[i];

	read_data(getPath("z"), buf, fileSize);
	REP(i, fileSize) m[i].z = buf[i];

	read_data(getPath("vx"), buf, fileSize);
	REP(i, fileSize) m[i].vx = buf[i];

	read_data(getPath("vy"), buf, fileSize);
	REP(i, fileSize) m[i].vy = buf[i];

	read_data(getPath("vz"), buf, fileSize);
	REP(i, fileSize) m[i].vz = buf[i];

	free(buf);
}

void printMatter(matter *m) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * fileSize);

	int i;

	REP(i, fileSize) buf[i] = m[i].x;
	write_data(buf, "ansx.double", fileSize);

	REP(i, fileSize) buf[i] = m[i].y;
	write_data(buf, "ansy.double", fileSize);

	REP(i, fileSize) buf[i] = m[i].z;
	write_data(buf, "ansz.double", fileSize);

	free(buf);
}

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nodeSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &nodeRank);
	createDataType();

	if ( argc != 4 ) {
		printf("this program argument format: ./exe [FileHeader] [FileHooter] [ElementSize]\n");
		return 1;
	}else {
		fileHeader = argv[1];
		fileHooter = argv[2];
		fileSize = atoi(argv[3]);
	}

	int arraySize = (fileSize / nodeSize) + ( fileSize % nodeSize > 0 ? 1 : 0 );

	matter m[arraySize * nodeSize];
	initMatter(m);

	matter cm[arraySize];

	int i, j, k;
	REP(k, STEP) {
		printf("STEP: %d, nodeRank: %d\n", k, nodeRank);

		double vx, vy, vz;
		double xx, yy, zz, r, rr, gm;

		int start = nodeRank * arraySize;
		int end = start + arraySize;
		end = end < fileSize ? end : fileSize;
		printf("nodeRank: %d, start: %d, end %d\n", nodeRank, start, end);
		int ci;
#pragma omp parallel for private(xx, yy, zz, r, rr, gm, vx, vy, vz, i, j, ci)
		for(i = start; i < end; ++i) {
			printf("STEP: %d, nodeRank: %d, i: %d\n", k, nodeRank, i);
			ci = i - start;

			cm[ci].m  = m[i].m;
			cm[ci].x  = m[i].x;
			cm[ci].y  = m[i].y;
			cm[ci].z  = m[i].z;
			cm[ci].vx = m[i].vx;
			cm[ci].vy = m[i].vy;
			cm[ci].vz = m[i].vz;

			vx = vy = vz = 0.0f;
			REP(j, fileSize) {
				if ( i != j ) {
					xx = m[i].x - m[j].x; xx *= xx;
					yy = m[i].y - m[j].y; yy *= yy;
					zz = m[i].z - m[j].z; zz *= zz;

					r = sqrt(xx + yy + zz);
					rr = r * r;

					gm = G * (m[i].m / rr);
					vx += gm * ((m[j].x - m[i].x) / r);
					vy += gm * ((m[j].y - m[i].y) / r);
					vz += gm * ((m[j].z - m[i].z) / r);
				}
			}

			cm[ci].vx += vx * DT;
			cm[ci].vy += vy * DT;
			cm[ci].vz += vz * DT;

			cm[ci].x += cm[ci].vx * DT;
			cm[ci].y += cm[ci].vy * DT;
			cm[ci].z += cm[ci].vz * DT;
		}
		printf("Message Passing start: %s, %s, %d\n", __FUNCTION__, __FILE__, __LINE__);

		MPI_Allgather(cm, arraySize, mpi_matter_type, m, arraySize, mpi_matter_type, MPI_COMM_WORLD);

		printf("Message Passing end:   %s, %s, %d\n", __FUNCTION__, __FILE__, __LINE__);
	}

	printf("%s, %s, %d\n", __FUNCTION__, __FILE__, __LINE__);
	if ( nodeRank == nodeSize - 1 ) printMatter(m);

	MPI_Type_free(&mpi_matter_type);
	MPI_Finalize();

	return 0;
}
