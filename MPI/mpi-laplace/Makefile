include $(BUILD)/config.make
include $(BUILD)/env/mpi
LANGUAGE = gnu-c
PROGRAM = laplace
PROGRAMS = $(PROGRAM) laplace-win
SRCS = laplace.c
OBJS = laplace.o
WINSRC = laplace-win.o
WINOBJS = laplace-win.o
CFLAGS =-O4
LDLIBS = -L$(X11LIBS) $(XLIB) -lm

INSTALL_BINDIR =

OBJDIR_RULE = prog-wrapper
include $(RULES)/srcdir

laplace-win:: laplace-win.o
	$(GXX) $(GXXFLAGS) $(GCCFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ laplace-win.o $(LDLIBS)
