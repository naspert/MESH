# $Id: Makefile,v 1.12 2001/09/03 15:36:48 aspert Exp $

#CC = /home/sun1/aspert/gcc/bin/gcc
CC = gcc
CPP = g++
BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj
GLDIR = /usr/X11R6
HPGLDIR = /usr/GL/hp



INCLFLAGS = -I../include
GLINCLFLAGS = -I$(HPGLDIR)/include -I$(GLDIR)/include
DEBUGINCFLAGS = -I/home/sun1/aspert/debug
XTRA_CFLAGS = -g  -Wall -O2  -ansi 

BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS) 
GL_CFLAGS = $(INCLFLAGS) $(GLINCLFLAGS) $(XTRA_CFLAGS) 

BASE_LIBFLAGS = -L$(LIBDIR) -lm
GL_LIBFLAGS = -L$(LIBDIR) -L$(HPGLDIR)/lib -L$(GLDIR)/lib -lglut -lGLU -lGL \
	-lXmu -lXext -lX11 -lXi -l3d -limage -lm

STATIC_GLFLAGS =   -L$(LIBDIR) -L$(GLDIR)/lib -lglut -lGLU -lGL -lXmu -lXext \
	-lX11 -lXi -lm 

BASE_LDFLAGS = -g $(BASE_LIBFLAGS) #-fbounds-checking
GL_LDFLAGS = -g $(GL_LIBFLAGS) #-fbounds-checking

default : rawview3

all: dirs rawview3 torus butterfly subdiv_sph

clean : 
	rm -f $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/*

rawview : $(OBJDIR)/rawview3.o $(OBJDIR)/geomutils.o $(OBJDIR)/normals.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/image.o $(OBJDIR)/gl2ps.o
	$(CC) $^ -o $(BINDIR)/$@ $(STATIC_GLFLAGS)

rawview3 : $(OBJDIR)/rawview3.o $(OBJDIR)/gl2ps.o lib3d libimage
	$(CC) $(OBJDIR)/rawview3.o $(OBJDIR)/gl2ps.o -o $(BINDIR)/$@ $(GL_LDFLAGS)

viewer : $(OBJDIR)/viewer.o lib3d libimage
	$(CPP) $< -o $(BINDIR)/$@ $(QTLIB) $(GL_LDFLAGS)

torus : $(OBJDIR)/torus.o lib3d 
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d

butterfly : $(OBJDIR)/butterfly.o lib3d
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d

loop : $(OBJDIR)/loop.o lib3d
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d

subdiv_sph : $(OBJDIR)/subdiv_sph.o lib3d
	$(CC) -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d

libimage : $(OBJDIR)/image.o
	$(CC) -g -shared  -o $(LIBDIR)/$@.so $<

lib3d : $(OBJDIR)/geomutils.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/normals.o
	$(CC) -g -shared -o $(LIBDIR)/$@.so $^


make_brep: $(OBJDIR)/make_brep.o lib3d
	$(CC) -g -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d


process_dist: $(OBJDIR)/process_dist.o lib3d
	$(CC) -g -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d

model_dist: $(OBJDIR)/model_dist.o lib3d
	$(CC) -g -o $(BINDIR)/$@ $< $(BASE_LDFLAGS) -l3d

$(OBJDIR)/viewer.o :viewer.cpp
	$(CPP) $(QTINCL) $(BASE_CFLAGS) -c $< -o $@


$(OBJDIR)/rawview3.o : rawview3.c
	$(CC) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/gl2ps.o : gl2ps.c
	$(CC) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c
	$(CC) $(BASE_CFLAGS) -c $< -o $@	


dirs : libdir bindir objdir

libdir : 
	-[ -d $(LIBDIR) ] || mkdir $(LIBDIR)

bindir : 
	-[ -d $(BINDIR) ] || mkdir $(BINDIR)

objdir :
	-[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
