# $Id: Makefile,v 1.7 2001/04/27 14:02:20 aspert Exp $

CC = gcc
CPP = g++
BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj
GLDIR = /usr/X11R6
HPGLDIR = /usr/GL/hp



INCLFLAGS = -I../include
QTINCL = -I/usr/lib/qt-2.3.0/include
QTLIB = -L/usr/lib/qt-2.3.0/lib -lqt
GLINCLFLAGS = -I$(HPGLDIR)/include -I$(GLDIR)/include
DEBUGINCFLAGS = -I/home/sun1/aspert/debug
XTRA_CFLAGS = -g  -Wall  -O3 

BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS)
GL_CFLAGS = $(INCLFLAGS) $(GLINCLFLAGS) $(XTRA_CFLAGS)

BASE_LIBFLAGS = -L$(LIBDIR) -lm
GL_LIBFLAGS = -L$(LIBDIR) -L$(HPGLDIR)/lib -L$(GLDIR)/lib -lglut -lGLU -lGL \
	-lXmu -lXext -lX11 -lXi -l3d -limage -lm

STATIC_GLFLAGS =  -L$(LIBDIR) -L$(GLDIR)/lib -lglut -lGLU -lGL -lXmu -lXext \
	-lX11 -lXi -lm  

BASE_LDFLAGS = -g -O3 $(BASE_LIBFLAGS) 
GL_LDFLAGS = -g -O3 $(GL_LIBFLAGS)

default : rawview3

all: dirs rawview3 torus subdiv 

clean : 
	rm $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/*

rawview : $(OBJDIR)/rawview3.o $(OBJDIR)/geomutils.o $(OBJDIR)/normals.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/image.o
	$(CC) $^ -o $(BINDIR)/rawview $(STATIC_GLFLAGS)

rawview3 : $(OBJDIR)/rawview3.o lib3d libimage
	$(CC) $(OBJDIR)/rawview3.o -o $(BINDIR)/rawview3 $(GL_LDFLAGS)

viewer : $(OBJDIR)/viewer.o lib3d libimage
	$(CPP) $(OBJDIR)/viewer.o -o $(BINDIR)/viewer $(QTLIB) $(GL_LDFLAGS)

torus : $(OBJDIR)/torus.o lib3d 
	$(CC) -o $(BINDIR)/torus $< $(BASE_LDFLAGS) -l3d

subdiv : $(OBJDIR)/subdiv.o lib3d
	$(CC) -o $(BINDIR)/subdiv $< $(BASE_LDFLAGS) -l3d

subdiv_sph : $(OBJDIR)/subdiv_sph.o lib3d
	$(CC) -o $(BINDIR)/subdiv_sph $< $(BASE_LDFLAGS) -l3d

libimage : $(OBJDIR)/image.o
	$(CC) -g -shared  -o $(LIBDIR)/libimage.so $<

lib3d : $(OBJDIR)/geomutils.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/normals.o
	$(CC) -g -shared -o $(LIBDIR)/lib3d.so $^

$(OBJDIR)/viewer.o :viewer.cpp
	$(CPP) $(QTINCL) $(BASE_CFLAGS) -c $< -o $@

$(OBJDIR)/rawview3.o : rawview3.c
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
