# $Id: Makefile,v 1.24 2001/11/30 13:55:08 aspert Exp $


OS := $(shell uname -s)
ARCH := $(shell uname -m)

GLXINFO = /usr/bin/glxinfo
STD_GLDIR = /usr/X11R6
HP_GLDIR = /usr/GL/hp
BASE_LIBFLAGS = -lm
TARGETS = torus cone isoca dirac compare_curv

ifeq ($(OS), Linux)
CC = gcc
XTRA_CFLAGS = -g  -pipe -ansi -pedantic -Wall -D__LINUX -D__USE_DISPLAY_LISTS_
BASE_LIBFLAGS += -lgsl -lgslcblas
TARGETS += maps_lsq
GL_VENDOR = $(shell $(GLXINFO)|grep "OpenGL vendor"|cut -d\: -f2|cut -d\  -f2)

ifeq ($(GL_VENDOR), Hewlett-Packard) # The HP visualize have special dirs.
GLDIR = $(HP_GLDIR)
STATIC_GLFLAGS =  -L$(GLDIR)/lib -lglut -lGLU -lGL -L$(STD_GLDIR)/lib -lX11 \
	-lXmu -lXext -lXi $(BASE_LIBFLAGS)
XTRA_CFLAGS += -Wno-unknown-pragmas # remove silly warnings of HP includes
else
GLDIR = $(STD_GLDIR)
STATIC_GLFLAGS =  -L$(GLDIR)/lib -lglut -lGLU -lGL -lX11 \
	-lXmu -lXext -lXi $(BASE_LIBFLAGS)
endif # HP
endif #Linux

ifeq ($(OS), IRIX64)
CC = cc
XTRA_CFLAGS = -g  -D__USE_DISPLAY_LISTS_
STATIC_GLFLAGS =  -lglut -lGLU -lGL -lX11 -lXmu $(BASE_LIBFLAGS)
endif

ifeq ($(ARCH), i686)
XTRA_CFLAGS += -march=i686 -fstrict-aliasing -ffast-math -fno-math-errno
endif


BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj

INCLFLAGS = -I../include
GLINCLFLAGS = -I$(GLDIR)/include
XTRA_CFLAGS += -O2

BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS) 
GL_CFLAGS = $(INCLFLAGS) $(GLINCLFLAGS) $(XTRA_CFLAGS) 

BASE_OBJECTS = $(OBJDIR)/geomutils.o $(OBJDIR)/normals.o $(OBJDIR)/3dmodel_io.o
GL_OBJECTS = $(OBJDIR)/rawview.o $(OBJDIR)/gl2ps.o

#
# Only for the dynamic stuff
#
GL_LIBFLAGS = -L$(LIBDIR) -L$(GLDIR)/lib -lglut -lGLU -lGL \
	-lXmu -lXext -lX11 -lXi -l3d -limage -lm 
BASE_LDFLAGS += -g $(BASE_LIBFLAGS)
GL_LDFLAGS += -g $(GL_LIBFLAGS) 
#
#

default : rawview

all: dirs rawview subdiv $(TARGETS)

rawview : $(GL_OBJECTS) $(BASE_OBJECTS) $(OBJDIR)/image.o
	$(CC) $^ -o $(BINDIR)/$@ $(STATIC_GLFLAGS)

$(TARGETS): % : $(OBJDIR)/%.o $(BASE_OBJECTS)
	$(CC) -o $(BINDIR)/$@ $^ $(BASE_LDFLAGS)

subdiv : $(OBJDIR)/subdiv.o $(OBJDIR)/subdiv_methods.o $(BASE_OBJECTS)
	$(CC) -o $(BINDIR)/$@ $^ $(BASE_LDFLAGS) 

rawview_d : $(GL_OBJECTS) lib3d libimage
	$(CC) $(GL_OBJECTS) -o $(BINDIR)/$@ $(GL_LDFLAGS)

libimage : $(OBJDIR)/image.o
	$(CC) -g -shared  -o $(LIBDIR)/$@.so $<

lib3d : $(BASE_OBJECTS)
	$(CC) -g -shared -o $(LIBDIR)/$@.so $^

$(GL_OBJECTS): $(OBJDIR)/%.o : %.c
	$(CC) $(GL_CFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c
	$(CC) $(BASE_CFLAGS) -c $< -o $@	


#
# Non-compile stuff below...
#
clean : 
	rm -f $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/*

dirs : libdir bindir objdir

libdir : 
	-[ -d $(LIBDIR) ] || mkdir $(LIBDIR)

bindir : 
	-[ -d $(BINDIR) ] || mkdir $(BINDIR)

objdir :
	-[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
