# $Id: Makefile,v 1.35 2002/04/22 10:27:24 aspert Exp $


OS := $(shell uname -s)
ARCH := $(shell uname -m)

GLXINFO = $(shell which glxinfo)
STD_GLDIR = /usr/X11R6
HP_GLDIR = /usr/GL/hp
BASE_LIBFLAGS = -lm -lz
TARGETS = torus cone dirac compare_curv
GSL_TARGETS = maps_lsq
ifeq ($(OS), Linux)
CC = gcc
XTRA_CFLAGS = -g  -pipe -ansi -pedantic -Wall -DREAD_TIME #-DDONT_USE_ZLIB \
	-D__DONT_USE_DISPLAY_LISTS_	


GL_VENDOR := $(shell $(GLXINFO)|grep "OpenGL vendor"|cut -d\: -f2|cut -d\  -f2)
ALL_TARGETS = $(TARGETS) rawview subdiv $(GSL_TARGETS)
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
XTRA_CFLAGS = -g  #-D__DONT_USE_DISPLAY_LISTS_
STATIC_GLFLAGS =  -lglut -lGLU -lGL -lX11 -lXmu $(BASE_LIBFLAGS)
ALL_TARGETS = $(TARGETS) rawview subdiv
endif

ifeq ($(ARCH), i686)
XTRA_CFLAGS += -march=i686 -fstrict-aliasing -ffast-math #-fno-math-errno
endif

#
# Output directories
#
BINDIR = ../bin
LIBDIR = ../lib
OBJDIR = ../obj

#
# Include flags
#
INCLFLAGS = -I../include
GLINCLFLAGS = -I$(GLDIR)/include
XTRA_CFLAGS += -O2
BASE_CFLAGS = $(INCLFLAGS) $(XTRA_CFLAGS) 
GL_CFLAGS = $(INCLFLAGS) $(GLINCLFLAGS) $(XTRA_CFLAGS) 

#
# Objects
#
BASE_OBJECTS = $(OBJDIR)/geomutils.o $(OBJDIR)/normals.o \
	$(OBJDIR)/3dmodel_io.o $(OBJDIR)/model_in.o
GL_OBJECTS = $(OBJDIR)/rawview.o $(OBJDIR)/gl2ps.o
GSL_OBJECTS = $(OBJDIR)/maps_lsq.o


#
# Linking flags
#
GSL_LDFLAGS =  -g $(BASE_LIBFLAGS) -lgsl -lgslcblas

#
# Linkign (only for the dynamic stuff)
#
GL_LIBFLAGS = -L$(LIBDIR) -L$(GLDIR)/lib -lglut -lGLU -lGL \
	-lXmu -lXext -lX11 -lXi -l3d -limage -lm 
BASE_LDFLAGS += -g $(BASE_LIBFLAGS)
GL_LDFLAGS += -g $(GL_LIBFLAGS) 


default : rawview

all: dirs $(ALL_TARGETS)

rawview : $(GL_OBJECTS) $(BASE_OBJECTS) $(OBJDIR)/image.o
	$(CC) $^ -o $(BINDIR)/$@ $(STATIC_GLFLAGS)

$(TARGETS): % : $(OBJDIR)/%.o $(BASE_OBJECTS)
	$(CC) -o $(BINDIR)/$@ $^ $(BASE_LDFLAGS)

$(GSL_TARGETS): % : $(OBJDIR)/%.o $(BASE_OBJECTS)
	$(CC) -o $(BINDIR)/$@ $^ $(GSL_LDFLAGS)

read: % : $(OBJDIR)/%.o $(OBJDIR)/model_in.o
	$(CC) -o $(BINDIR)/$@ $^ $(BASE_LDFLAGS)

isoca: $(OBJDIR)/isoca.o $(OBJDIR)/subdiv.o $(BASE_OBJECTS)
	$(CC) -o $(BINDIR)/$@ $^ $(BASE_LDFLAGS) 

subdiv : $(OBJDIR)/subdiv_main.o $(OBJDIR)/subdiv.o \
	$(OBJDIR)/subdiv_methods.o $(BASE_OBJECTS)
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
