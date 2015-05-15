/*
 *	Laplace Equation with Visualizer
 *	Copyright 1999 Real World Computing Partnership
 *			Written by Yutaka Ishikawa, ishikawa@rwcp.or.jp
 *
 *	This example is used in the RWCP Video
 */
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <X11/Xlib.h>

#define YSIZE	32
#define XSIZE	128
#define EPS	1.0e-6
#define SCALE	4
#define	WIDTH	(XSIZE*SCALE)
#define HEIGHT	(YSIZE*SCALE)
#define CLRD	8
#define CLRSIZE	(256*CLRD)

double		u[YSIZE][XSIZE];
double		uu[YSIZE][XSIZE];
int		myPE;
int		color[CLRSIZE];
int		fbuffer[WIDTH*HEIGHT];
Display		*dp;
Window		wid;
GC		gc;
XImage		*xim;

void		init(double u[YSIZE][XSIZE]);
extern int	xinit(int, char**);
extern void	xupdate(int, int);

int
main(int argc, char **argv)
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

    /**/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myPE);
    /* X initialize */
    if (xinit(argc, argv) < 0) {
	MPI_Finalize();
	exit(-1);
    }
    /* Setup local parameters */
    myStart = (YSIZE/nprocs)*myPE;
    size = YSIZE/nprocs;
    if (myPE == 0) { myStart++; --size; }
    if (myPE == (nprocs - 1)) --size;
    /**/
    init(u);
    init(uu);
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
	xupdate(myStart - 1, YSIZE/nprocs);
    } while (err > EPS);
#ifdef not
    if (myPE == 0) {
	for (i = myStart + size - 1; i < myStart + size; i++) {
	    for (j = 1; j < XSIZE - 1; j += 4) {
		printf("%e %e %e %e\n",
		       u[i][j], u[i][j + 1], u[i][j + 2], u[i][j + 3]);
	    }
	}
    }
#endif /* not */
    MPI_Finalize();
    exit(0);
}


void
xupdate(int myStart, int size)
{
    int		i, j, k, l;

    for (i = myStart; i < myStart + size; i++)
	for (j = 0; j < XSIZE; j++) {
	    double	vlog = log10(u[i][j] * 10000000000.0);
	    int		val = (int) ((vlog/10.0) * (CLRSIZE - 1));
	    if (val < 0) val = 0;
	    for (k = 0; k < SCALE; k++) {
		for (l = 0; l < SCALE; l++) {
		    fbuffer[(i*SCALE + k)*XSIZE*SCALE + j*SCALE + l]
			= color[val];
		}
	    }
	}
    MPI_Gather(&fbuffer[myStart*XSIZE*SCALE*SCALE],
	       size*XSIZE*SCALE*SCALE, MPI_INT,
	       fbuffer, size*XSIZE*SCALE*SCALE, MPI_INT, 0, MPI_COMM_WORLD);
    if (myPE == 0) {
	XPutImage(dp, wid, gc, xim, 0, 0, 0, 0, WIDTH, HEIGHT);
	XFlush(dp);
    }
}

int
xinit(int argc, char **argv)
{
    int		i, j;
    Window	parent;
    int		x, y;
    unsigned	width, height, border_width;
    int		depth;
    unsigned	border, background;
    XGCValues	gcv;

    for (i = 0; i < CLRSIZE/CLRD; i++) {
	for (j = 0; j < CLRD; j++) {
	    color[i*CLRD + j] = ((i&0xff) << 16) | ((~i) & 0xff);
	}
    }
    if (myPE != 0) {
	return 0;
    }
    if (argc != 2) {
	fprintf(stderr, "%s <hostname>:0\n", argv[0]);
	return -1;
    }
    /* open server */
    if(!(dp = XOpenDisplay(argv[1]))) {
	fprintf(stderr, "Cannot conntect %s\n", argv[1]);
	return -1;
    }
    /* create simple window */
    parent = DefaultRootWindow(dp);
    x = 0; y = 0;
    width = WIDTH;
    height = HEIGHT;
    border_width = 3;
    border = BlackPixel(dp, 0);
    background = WhitePixel(dp, 0);
    wid = XCreateSimpleWindow(dp, parent, x, y, width, height,
			      border_width, border, background);
    /* create graphic context */
    gcv.function= GXcopy;
    gcv.foreground = BlackPixel(dp, 0);
    gcv.background = WhitePixel(dp, 0);
    gc = XCreateGC(dp, wid, GCFunction|GCForeground|GCBackground, &gcv);
    XMapWindow(dp, wid);
    XFlush(dp);

    xim = XCreateImage(dp, 0, DisplayPlanes(dp, DefaultScreen(dp)),
		       ZPixmap, 0, NULL, width, height, XBitmapPad(dp), 0);
    xim->data = (char*) fbuffer;
    return 0;
}

void
init(double u[YSIZE][XSIZE])
{
    int		i, j;

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
