#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include "data_util_bin.c"

#include <omp.h>

#define STEP 10
#define DT 1.0f
#define G 1.0f

#define REP(i, n) for(i=0;i<n;++i)
int fileSize = 0;
char* fileHeader;
char* fileHooter;

void printThreads() {
	int count;
	#pragma omp parallel
	{
		count = omp_get_num_threads();
	}
	printf("Use threads: %2d\n", count);
}

typedef struct matter {
	double m, x, y, z, vx, vy, vz;
}matter;

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

int main(int argc, char* argv[]) {
	if ( argc != 4 ) {
		printf("this program argument format: ./exe [FileHeader] [FileHooter] [ElementSize]\n");
		return 1;
	}else {
		fileHeader = argv[1];
		fileHooter = argv[2];
		fileSize = atoi(argv[3]);
	}

	matter m[2][fileSize];
	initMatter(m[0]);

	printThreads();

	int i, j, k;
	REP(k, STEP) {
		printf("%s, %s, %d, STEP: %d\n", __FUNCTION__, __FILE__, __LINE__, k);

		int p = k&1;
		int n = (k+1)&1;

		REP(i, fileSize) {
			m[n][i].m  = m[p][i].m;
			m[n][i].x  = m[p][i].x;
			m[n][i].y  = m[p][i].y;
			m[n][i].z  = m[p][i].z;
			m[n][i].vx = m[p][i].vx;
			m[n][i].vy = m[p][i].vy;
			m[n][i].vz = m[p][i].vz;
		}

		// private value
		double xx, yy, zz, r, rr, gm;
		#pragma omp parallel for private(xx, yy, zz, r, rr, gm, i, j)
		REP(i, fileSize) {
			REP(j, fileSize) {
				if ( i != j ) {
					xx = m[p][i].x - m[p][j].x; xx *= xx;
					yy = m[p][i].y - m[p][j].y; yy *= yy;
					zz = m[p][i].z - m[p][j].z; zz *= zz;

					r = 1.0 / sqrt(xx + yy + zz);
					rr = r * r;

					gm = G * (m[p][i].m * rr);
					m[n][i].vx += gm * ((m[p][j].x - m[p][i].x) * r);
					m[n][i].vy += gm * ((m[p][j].y - m[p][i].y) * r);
					m[n][i].vz += gm * ((m[p][j].z - m[p][i].z) * r);
				}
			}

			m[n][i].x += m[n][i].vx * DT;
			m[n][i].y += m[n][i].vy * DT;
			m[n][i].z += m[n][i].vz * DT;
		}
	}

	printf("%s, %s, %d\n", __FUNCTION__, __FILE__, __LINE__);
	printMatter(m[STEP&1]);

	return 0;
}
