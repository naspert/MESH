# $Id: Makefile,v 1.4 2001/03/13 12:43:32 aspert Exp $

CC = gcc

BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj
GLDIR = /usr/X11R6
HPGLDIR = /usr/GL/hp

INCLFLAGS = -I../include
GLINCLFLAGS = -I$(HPGLDIR)/include -I$(GLDIR)/include
DEBUGINCFLAGS = -I/home/sun1/aspert/debug
XTRA_CFLAGS = -g  -Wall  -O3

BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS)
GL_CFLAGS = $(INCLFLAGS) $(GLINCLFLAGS) $(XTRA_CFLAGS)

BASE_LIBFLAGS = -L$(LIBDIR) -lm
GL_LIBFLAGS = -L$(LIBDIR) -L$(HPGLDIR)/lib -L$(GLDIR)/lib -lglut -lGLU -lGL \
	-lXmu -lXext -lX11 -lXi -l3d -limage -lm

BASE_LDFLAGS = -g -O3 $(BASE_LIBFLAGS)
GL_LDFLAGS = -g -O3 $(GL_LIBFLAGS)

default : rawview3

all: rawview3 torus

rawview3 : $(OBJDIR)/rawview3.o lib3d libimage
	$(CC) $(OBJDIR)/rawview3.o -o $(BINDIR)/rawview3 $(GL_LDFLAGS)

libimage : $(OBJDIR)/image.o
	$(CC) -g -shared  -o $(LIBDIR)/libimage.so $<

lib3d : $(OBJDIR)/geomutils.o $(OBJDIR)/3dmodel_io.o $(OBJDIR)/normals.o
	$(CC) -g -shared -o $(LIBDIR)/lib3d.so $^

torus : $(OBJDIR)/torus.o lib3d 
	$(CC) -o $(BINDIR)/torus $< $(BASE_LDFLAGS) -l3d

$(OBJDIR)/rawview3.o : rawview3.c
	$(CC) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c
	$(CC) $(BASE_CFLAGS) -c $< -o $@	

clean : 
	rm $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/*

