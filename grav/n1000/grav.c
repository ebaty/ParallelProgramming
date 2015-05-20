#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include "data_util_bin.c"

#define STEP 10
#define DT 1.0f
#define G 1.0f
#define DEFAULT_SIZE 1000

#define REP(i, n) for(i=0;i<n;++i)
#define PATH ""

typedef struct matter {
	double m, x, y, z, vx, vy, vz;
}matter;

int initMatter(matter *p) {
	double *buf;
	buf = (double *)malloc(sizeof(double) * DEFAULT_SIZE);

	int i;

	read_data(strcat(PATH, "m.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].m = buf[i];

	read_data(strcat(PATH, "x.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].x = buf[i];

	read_data(strcat(PATH, "y.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].y = buf[i];

	read_data(strcat(PATH, "z.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].z = buf[i];

	read_data(strcat(PATH, "vx.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].vx = buf[i];

	read_data(strcat(PATH, "vy.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].vy = buf[i];

	read_data(strcat(PATH, "vz.double"), buf, DEFAULT_SIZE);
	for(i = 0; i < DEFAULT_SIZE; ++i) p[i].vz = buf[i];

	free(buf);
}

int main(void) {
	matter m[2][DEFAULT_SIZE];
	initMatter(m[0]);

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
	return 0;
}
