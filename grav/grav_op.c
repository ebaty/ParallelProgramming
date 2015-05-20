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
#define DEFAULT_SIZE 10000

#define REP(i, n) for(i=0;i<n;++i)
#define kPathHeader "n10000/"
#define kPathHooter ".double"

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

char* getPath(char *s) {
	char header[] = kPathHeader;
	char hooter[] = kPathHooter;

	return strcat(strcat(header, s), hooter);
}

void initMatter(matter *m) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * DEFAULT_SIZE);

	int i;

	read_data(getPath("m"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].m = buf[i];

	read_data(getPath("x"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].x = buf[i];

	read_data(getPath("y"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].y = buf[i];

	read_data(getPath("z"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].z = buf[i];

	read_data(getPath("vx"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].vx = buf[i];

	read_data(getPath("vy"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].vy = buf[i];

	read_data(getPath("vz"), buf, DEFAULT_SIZE);
	REP(i, DEFAULT_SIZE) m[i].vz = buf[i];

	free(buf);
}

void printMatter(matter *m) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * DEFAULT_SIZE);

	int i;

	REP(i, DEFAULT_SIZE) buf[i] = m[i].x;
	write_data(buf, "ansx.double", DEFAULT_SIZE);

	REP(i, DEFAULT_SIZE) buf[i] = m[i].y;
	write_data(buf, "ansy.double", DEFAULT_SIZE);

	REP(i, DEFAULT_SIZE) buf[i] = m[i].z;
	write_data(buf, "ansz.double", DEFAULT_SIZE);
}

int main(void) {
	matter m[2][DEFAULT_SIZE];
	initMatter(m[0]);

	printThreads();

	int i, j, k;
	REP(k, STEP) {
		REP(i, DEFAULT_SIZE) {
			int p = k&1;
			int n = (k+1)&1;
			m[n][i].m  = m[p][i].m;
			m[n][i].x  = m[p][i].x;
			m[n][i].y  = m[p][i].y;
			m[n][i].z  = m[p][i].z;
			m[n][i].vx = m[p][i].vx;
			m[n][i].vy = m[p][i].vy;
			m[n][i].vz = m[p][i].vz;

			double vx, vy, vz;
			vx = vy = vz = 0;

			#pragma omp parallel for reduction(+:vx, vy, vz)
			REP(j, DEFAULT_SIZE) {
				if ( i == j ) continue;

				double xx = m[p][i].x - m[p][j].x; xx *= xx;
				double yy = m[p][i].y - m[p][j].y; yy *= yy;
				double zz = m[p][i].z - m[p][j].z; zz *= zz;

				double r = sqrt(xx + yy + zz);
				double rr = r * r;

				double gm = G * (m[p][i].m / rr);
				vx += gm * ((m[p][j].x - m[p][i].x) / r);
				vy += gm * ((m[p][j].y - m[p][i].y) / r);
				vz += gm * ((m[p][j].z - m[p][i].z) / r);
			}

			m[n][i].vx += vx * DT;
			m[n][i].vy += vy * DT;
			m[n][i].vz += vz * DT;

			m[n][i].x += m[n][i].vx * DT;
			m[n][i].y += m[n][i].vy * DT;
			m[n][i].z += m[n][i].vz * DT;
		}
	}

	printMatter(m[(STEP-1)&1]);

	return 0;
}
