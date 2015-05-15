/*
 *	Laplace Equation
 *	Copyright 1999 Real World Computing Partnership
 *			Written by Yutaka Ishikawa, ishikawa@rwcp.or.jp
 *
 *	This example is used in the RWCP Video
 */

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <X11/Xlib.h>

#define YSIZE	1024
#define XSIZE	1024
#define EPS		1.0e-4

 double		u[YSIZE][XSIZE];
 double		uu[YSIZE][XSIZE];
 int			myPE;

 void init(double u[YSIZE][XSIZE]);

 int main(int argc, char **argv)
 {
 	int		i, j;
 	double	err, diff;
 	int		iter = 0;
 	int		nprocs;
 	int		size;
 	int		myStart;
 	MPI_Request	req1;
 	MPI_Request req2;
 	MPI_Status	status;
 	double	start, end;
 	double	time, maxtime;

    /**/
 	MPI_Init(&argc, &argv);
 	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
 	MPI_Comm_rank(MPI_COMM_WORLD, &myPE);

    /* Setup local parameters */
 	myStart = (YSIZE / nprocs) * myPE;
 	size = YSIZE / nprocs;
 	if (myPE == 0) {
 		myStart++;
 		--size;
 	}
 	if (myPE == (nprocs - 1)) --size;
 	
    /**/
 	init(u);
 	init(uu);
 	start = MPI_Wtime();
 	do {
 		double		lerr = 0.0;

 		iter++;
	/* copy */
 		for (i = myStart - 1; i < myStart + size + 1; i++)
 			for (j = 1; j < XSIZE - 1; j++)
 				uu[i][j] = u[i][j];
	/* update */
 			for (i = myStart; i < myStart + size; i++)
 				for (j = 1; j < XSIZE - 1; j++)
 					u[i][j] = (uu[i - 1][j] + uu[i + 1][j]
 						+ uu[i][j - 1] + uu[i][j + 1])/4.0;
	/* exchange */
	if (myPE != 0) { /* Except the first processor */
 				MPI_Irecv(&u[myStart - 1][1], XSIZE - 2, MPI_DOUBLE,
 					myPE - 1, 0, MPI_COMM_WORLD, &req1);
 			}
	if (myPE != (nprocs - 1)) { /* Except the last processor */
 			MPI_Irecv(&u[myStart + size][1], XSIZE - 2, MPI_DOUBLE,
 				myPE + 1, 0, MPI_COMM_WORLD, &req2);
 		}
	if (myPE != 0) { /* Except the first processor */
 		MPI_Send(&u[myStart][1], XSIZE - 2, MPI_DOUBLE,
 			myPE - 1, 0, MPI_COMM_WORLD);
 	}
	if (myPE != (nprocs - 1)) { /* Except the last processor */
 	MPI_Send(&u[myStart + size - 1][1], XSIZE - 2, MPI_DOUBLE,
 		myPE + 1, 0, MPI_COMM_WORLD);
 }
 if (myPE != 0) MPI_Wait(&req1, &status);
 if (myPE != (nprocs - 1)) MPI_Wait(&req2, &status);
	/* error */
 lerr = 0.0;
 for (i = myStart; i < myStart + size; i++)
 	for (j = 1; j < XSIZE - 1; j++) {
 		diff = uu[i][j] - u[i][j];
 		lerr += diff*diff;
 	}
 	MPI_Allreduce(&lerr, &err, 1, MPI_DOUBLE,
 		MPI_SUM, MPI_COMM_WORLD);
 } while (err > EPS);

 end = MPI_Wtime();
 time = end - start;
 MPI_Allreduce(&time, &maxtime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
 if (myPE == 0) {
 	printf("iteration = %d time = %f seconds\n", iter, maxtime);
 }
 MPI_Finalize();
 exit(0);
}

void init(double u[YSIZE][XSIZE])
{
	int	i, j;

	for (i = 0; i < YSIZE; i++) {
		for (j = 0; j < XSIZE; j++) {
			u[i][j] = 0.0;
		}
	}
	for (i = 0; i < YSIZE; i++) {
		u[i][0] = 1.0;
		u[i][XSIZE - 1] = 0.0;
	}
	for (j = 0; j < XSIZE; j++) {
		u[0][j] = 0.0;
		u[YSIZE - 1][j] = 0.0;
	}
}
