#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "data_util.h"

void init_location(double *loc, int size)
{
	int i;

	for (i = 0; i < size; i++) {
			loc[i] = rand() / (RAND_MAX + 1.0) * 100.0 - 50.0;
		}
}

void init_mass(double *mass, int size)
{
	int i;

	for(i = 0; i < size; i++) {
			mass[i] = 1.0 + rand() / (RAND_MAX + 1.0);
		}
}

void init_velocity(double *vel, int size)
{
	init_location(vel, size);
}

int main(int argc, char **argv)
{
	int size;
	double *m, *x, *y, *z, *vx, *vy, *vz;

	if (argc == 1) size = DEFAULT_SIZE;
	else size = atoi(argv[1]);

	m  = (double *)malloc(sizeof(double) * size);
	x  = (double *)malloc(sizeof(double) * size);
	y  = (double *)malloc(sizeof(double) * size);
	z  = (double *)malloc(sizeof(double) * size);
	vx = (double *)malloc(sizeof(double) * size);
	vy = (double *)malloc(sizeof(double) * size);
	vz = (double *)malloc(sizeof(double) * size);

	init_mass(m, size);
	init_location(x, size);
	init_location(y, size);
	init_location(z, size);
	init_velocity(vx, size);
	init_velocity(vy, size);
	init_velocity(vz, size);

	write_data(m, "m.dat", size);
	write_data(x, "x.dat", size);
	write_data(y, "y.dat", size);
	write_data(z, "z.dat", size);
	write_data(vx, "vx.dat", size);
	write_data(vy, "vy.dat", size);
	write_data(vx, "vz.dat", size);

	read_data("x.dat", x, size);

	free(m);
	free(x);
	free(y);
	free(z);
	free(vx);
	free(vy);
	free(vz);

	printf("Making data successed.\n");
	return 0;
}
