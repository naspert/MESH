# $Id: Makefile,v 1.22 2001/11/23 13:49:03 aspert Exp $

#CC = /home/sun1/aspert/gcc/bin/gcc

OS := $(shell uname -s)
ARCH := $(shell uname -m)

ifeq ($(OS),Linux)
CC = gcc
XTRA_CFLAGS = -g  -pipe -ansi -pedantic -Wall -D__LINUX #-D__USE_DISPLAY_LISTS_
endif
ifeq ($(OS),IRIX64)
CC = cc
XTRA_CFLAGS = -g  -D__USE_DISPLAY_LISTS_
endif
ifeq ($(ARCH),i686)
XTRA_CFLAGS += -march=i686 -fstrict-aliasing -ffast-math -fno-math-errno
endif

BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj
GLDIR = /usr/X11R6
HPGLDIR = /usr/GL/hp



INCLFLAGS = -I../include
GLINCLFLAGS = -I$(HPGLDIR)/include -I$(GLDIR)/include
XTRA_CFLAGS +=  -O2  #-D__RING_DEBUG

BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS) 
GL_CFLAGS = $(INCLFLAGS) $(GLINCLFLAGS) $(XTRA_CFLAGS) 

BASE_LIBFLAGS = -L$(LIBDIR) -l3d -lm 
GL_LIBFLAGS = -L$(LIBDIR) -L$(HPGLDIR)/lib -L$(GLDIR)/lib -lglut -lGLU -lGL \
	-lXmu -lXext -lX11 -lXi -l3d -limage -lm 

STATIC_GLFLAGS =   -L$(LIBDIR) -L$(GLDIR)/lib -lglut -lGLU -lGL -lXmu -lXext \
	-lX11 -lXi -lm 

BASE_LDFLAGS += -g $(BASE_LIBFLAGS) #-pg
GL_LDFLAGS += -g $(GL_LIBFLAGS) 


default : rawview3

all: dirs rawview3 torus cone isoca  subdiv dirac compare_curv

clean : 
	rm -f $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/*

rawview : $(OBJDIR)/rawview3.o $(OBJDIR)/geomutils.o $(OBJDIR)/normals.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/image.o $(OBJDIR)/gl2ps.o
	$(CC) $^ -o $(BINDIR)/$@ $(STATIC_GLFLAGS)

rawview3 : $(OBJDIR)/rawview3.o $(OBJDIR)/gl2ps.o lib3d libimage
	$(CC) $(OBJDIR)/rawview3.o $(OBJDIR)/gl2ps.o -o $(BINDIR)/$@ $(GL_LDFLAGS)

torus : $(OBJDIR)/torus.o lib3d 
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) 

isoca : $(OBJDIR)/isoca.o lib3d
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) 

cone: $(OBJDIR)/cone.o lib3d
	$(CC) -g -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) 

dirac: $(OBJDIR)/dirac.o lib3d
	$(CC) -g -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) 

maps_lsq : $(OBJDIR)/maps_lsq.o lib3d
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS)  -lgsl -lgslcblas

compare_curv : $(OBJDIR)/compare_curv.o lib3d
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) 

subdiv : $(OBJDIR)/subdiv.o $(OBJDIR)/subdiv_methods.o lib3d
	$(CC) -o $(BINDIR)/$@ $(OBJDIR)/subdiv.o $(OBJDIR)/subdiv_methods.o \
	 $(BASE_LDFLAGS) 

libimage : $(OBJDIR)/image.o
	$(CC) -g -shared  -o $(LIBDIR)/$@.so $<

lib3d : $(OBJDIR)/geomutils.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/normals.o
	$(CC) -g -shared -o $(LIBDIR)/$@.so $^

$(OBJDIR)/rawview3.o : rawview3.c
	$(CC) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/gl2ps.o : gl2ps.c
	$(CC) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/purify.o : purify.c
	$(CC) $(BASE_P_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c
	$(CC) $(BASE_CFLAGS) -c $< -o $@	


dirs : libdir bindir objdir

libdir : 
	-[ -d $(LIBDIR) ] || mkdir $(LIBDIR)

bindir : 
	-[ -d $(BINDIR) ] || mkdir $(BINDIR)

objdir :
	-[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
