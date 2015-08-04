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

char fileName[1024];
char* getPath(char *s) {
	int i;
	sprintf(fileName, "%s%s%s", fileHeader, s, fileHooter);

	return fileName;
}

void initMatter(int offset, matter *m, int arraySize) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * fileSize);

	int i;

	read_data(getPath("m"), buf, fileSize);
	REP(i, arraySize) m[i].m = buf[offset+i];

	read_data(getPath("x"), buf, fileSize);
	REP(i, arraySize) m[i].x = buf[offset+i];

	read_data(getPath("y"), buf, fileSize);
	REP(i, arraySize) m[i].y = buf[offset+i];

	read_data(getPath("z"), buf, fileSize);
	REP(i, arraySize) m[i].z = buf[offset+i];

	read_data(getPath("vx"), buf, fileSize);
	REP(i, arraySize) m[i].vx = buf[offset+i];

	read_data(getPath("vy"), buf, fileSize);
	REP(i, arraySize) m[i].vy = buf[offset+i];

	read_data(getPath("vz"), buf, fileSize);
	REP(i, arraySize) m[i].vz = buf[offset+i];

	free(buf);
}

void printMatter(matter *m, int nodeRank, int arraySize) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * arraySize);

	char fileName[1024];

	int i;
	REP(i, arraySize) buf[i] = m[i].x;
	sprintf(fileName, "node%d_ansx.double", nodeRank);
	write_data(buf, fileName, arraySize);

	REP(i, arraySize) buf[i] = m[i].y;
	sprintf(fileName, "node%d_ansy.double", nodeRank);
	write_data(buf, fileName, arraySize);

	REP(i, arraySize) buf[i] = m[i].z;
	sprintf(fileName, "node%d_ansz.double", nodeRank);
	write_data(buf, fileName, arraySize);

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

	omp_set_nested(1);
	int arraySize = (fileSize / nodeSize) + ( fileSize % nodeSize > 0 ? 1 : 0 );

	matter m[arraySize];
	initMatter(nodeRank * arraySize, m, arraySize);

	matter buf[2][arraySize];

	double time = MPI_Wtime();

	int i, j, buf_j, k;
	REP(k, STEP) {
		printf("STEP: %d, nodeRank: %d\n", k, nodeRank);

		REP(i, arraySize) buf[0][i] = m[i];
		REP(i, nodeSize) {
			int bufIndex = i & 1;
			#pragma omp parallel sections 
			{
				#pragma omp section
				{
						if ( i < nodeSize-1 ) {
							MPI_Status status;
							int sendRank = (nodeRank - (i + 1) + nodeSize) % nodeSize;
							int recvRank = (nodeRank + (i + 1) ) % nodeSize;
							MPI_Sendrecv(									 m, arraySize, mpi_matter_type, sendRank, 0,
													 buf[(bufIndex+1)&1], arraySize, mpi_matter_type, recvRank, 0, MPI_COMM_WORLD, &status);
						}
				}
				#pragma omp section
				{
					double xx, yy, zz, r, rr, gm;
					#pragma omp parallel for private(xx, yy, zz, r, rr, gm, j, buf_j) num_threads(15)
					REP(j, arraySize) REP(buf_j, arraySize) {
						if ( (i != 0) || (j != buf_j) ) {
							xx = m[j].x - buf[bufIndex][buf_j].x; xx *= xx;
							yy = m[j].y - buf[bufIndex][buf_j].y; yy *= yy;
							zz = m[j].z - buf[bufIndex][buf_j].z; zz *= zz;

							r = 1.0 / sqrt(xx + yy + zz);
							rr = r * r;

							gm = G * (m[j].m * rr);
							m[j].vx += gm * ((buf[bufIndex][buf_j].x - m[j].x) * r);							
							m[j].vy += gm * ((buf[bufIndex][buf_j].y - m[j].y) * r);							
							m[j].vz += gm * ((buf[bufIndex][buf_j].z - m[j].z) * r);							
						}
					}
				}
			}
		}
		REP(i, arraySize) {
			m[i].x += m[i].vx * DT;
			m[i].y += m[i].vy * DT;
			m[i].z += m[i].vz * DT;
		}
	}

	time = MPI_Wtime() - time;
	
	printMatter(m, nodeRank, arraySize);
	printf("%s, %s, %d\n", __FUNCTION__, __FILE__, __LINE__);

	MPI_Type_free(&mpi_matter_type);
	MPI_Finalize();

	if ( nodeRank == 0 ) printf("time: %lf\n", time);

	return 0;
}
