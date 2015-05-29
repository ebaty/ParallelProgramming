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

void matterAllgather(matter *send, matter *receive) {
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Allgather(send, arraySize, mpi_matter_type, receive, arraySize, mpi_matter_type, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
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
}

int main(int argc, char *argv[]) {
	if ( argc != 4 ) {
		printf("this program argument format: ./exe [FileHeader] [FileHooter] [ElementSize]\n");
		return 1;
	}else {
		fileHeader = argv[1];
		fileHooter = argv[2];
		fileSize = atoi(argv[3]);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nodeSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &nodeRank);
	createDataType();

	int arraySize = (fileSize / nodeSize);
	if ( fileSize % nodeSize ) {
		arraySize += 1;
	}

	matter m[arraySize * nodeSize];
	initMatter(m);

	matter cm[arraySize];

	int i, j, k;
	REP(k, STEP) {
		/* printf("STEP: %d, nodeRank: %d\n", k, nodeRank); */

		for(i = nodeRank * arraySize; i < ((nodeRank + 1) * arraySize) && i < fileSize; ++i) {
			/* printf("STEP: %d, nodeRank: %d, i: %d\n", k, nodeRank, i); */
			cm[i].m  = m[i].m;
			cm[i].x  = m[i].x;
			cm[i].y  = m[i].y;
			cm[i].z  = m[i].z;
			cm[i].vx = m[i].vx;
			cm[i].vy = m[i].vy;
			cm[i].vz = m[i].vz;

			double vx, vy, vz;
			vx = vy = vz = 0;

			// private value
			double xx, yy, zz, r, rr, gm;
			/* #pragma omp parallel for private(xx, yy, zz, r, rr, gm) */
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

			cm[i].vx += vx * DT;
			cm[i].vy += vy * DT;
			cm[i].vz += vz * DT;

			cm[i].x += cm[i].vx * DT;
			cm[i].y += cm[i].vy * DT;
			cm[i].z += cm[i].vz * DT;
		}

		matterAllgather(cm, m);
	}

	printMatter(m);

	MPI_Type_free(&mpi_matter_type);
	MPI_Finalize();

	return 0;
}
