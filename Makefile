# $Id: Makefile,v 1.4 2001/06/01 09:43:11 jacquet Exp $

CC = gcc
CPP = g++



BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj
LIB3DDIR = /home/sun1/jacquet/lib3d
QTDIR = /usr/lib/qt-2.3.0
MOC = $(QTDIR)/bin/moc



INCLFLAGS = -I$(LIB3DDIR)/include -I. 
QTINCFLAGS = $(INCLFLAGS) -I$(QTDIR)/include
GLINCLFLAGS = $(INCLFLAGS) -I/usr/X11R6/include
DEBUGINCFLAGS = -I/home/sun1/aspert/debug
XTRA_CFLAGS = -g -O2 

BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS)
GL_CFLAGS = $(GLINCLFLAGS) $(XTRA_CFLAGS)

LIB3D_FLAGS = -L$(LIB3DDIR)/lib -l3d #-limage
BASE_LIBFLAGS = -L$(LIBDIR)  -lm
GL_LIBFLAGS = -L$(LIBDIR) -L/usr/X11R6/lib  -lglut -lGLU -lGL -lXmu -lXext -lX11 $(LIB3D_FLAGS) -lm


BASE_LDFLAGS = -g -O2  $(BASE_LIBFLAGS)
GL_LDFLAGS = -g -O2 $(GL_LIBFLAGS)
QT_LIBFLAGS = -L$(QTDIR)/lib -lqt
QTGL_LIBFLAGS = $(QT_LIBFLAGS) -L/usr/X11R6/lib -lGLU -lGL $(LIB3D_FLAGS) -lm


default : viewer

all: dirs viewer 

clean : 
	rm $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/* $(LIB3DDIR)/obj/* $(LIB3DDIR)/lib/*

viewer : $(OBJDIR)/viewer.o $(OBJDIR)/ScreenWidget.o $(OBJDIR)/compute_error.o $(OBJDIR)/RawWidget.o $(OBJDIR)/moc_RawWidget.o $(OBJDIR)/ColorMapWidget.o $(OBJDIR)/ColorMap.o lib3d
	$(CPP) -O2 $(OBJDIR)/viewer.o $(OBJDIR)/ScreenWidget.o $(OBJDIR)/compute_error.o $(OBJDIR)/RawWidget.o $(OBJDIR)/moc_RawWidget.o $(OBJDIR)/ColorMapWidget.o $(OBJDIR)/ColorMap.o -o ../viewer $(QTGL_LIBFLAGS)

lib3d :  $(LIB3DDIR)/obj/3dmodel_io.o $(LIB3DDIR)/obj/normals.o  $(LIB3DDIR)/obj/geomutils.o
	$(CC) -g -shared -o $(LIB3DDIR)/lib/lib3d.so $^

$(OBJDIR)/viewer.o : viewer.cpp
	$(CPP) -D_METRO -O2 -ansi -lm $(QTINCFLAGS) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/moc_RawWidget.o : moc_RawWidget.cpp
	$(CPP) -D_METRO -O2 -ansi -lm $(QTINCFLAGS) $(GL_CFLAGS) -c $< -o $@

moc_RawWidget.cpp : RawWidget.h
	$(MOC) -D_METRO $< -o $@

$(LIB3DDIR)/obj/%.o : $(LIB3DDIR)/src/%.c
	$(CC) $(BASE_CFLAGS) -D_METRO -c $< -o $@

$(OBJDIR)/%.o : %.c
	$(CC)  $(BASE_CFLAGS) -D_METRO -c $< -o $@	

$(OBJDIR)/%.o : %.cpp
	$(CPP) $(BASE_CFLAGS) $(QTINCFLAGS) -D_METRO -c $< -o $@




dirs : libdir bindir objdir

libdir : 
	-[ -d $(LIBDIR) ] || mkdir $(LIBDIR)

bindir : 
	-[ -d $(BINDIR) ] || mkdir $(BINDIR)

objdir :
	-[ -d $(OBJDIR) ] || mkdir $(OBJDIR)