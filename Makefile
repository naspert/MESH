# $Id: Makefile,v 1.12 2001/08/08 13:05:02 dsanta Exp $

#
# If the make variable PROFILE is defined, profiling flags are automatically
# added. If the value of the variable is 'full' (without the quotes) the
# executable is linked with the profiling versions of the standard C library.
# Note that 'full' requires the creation of a fully static executable, and
# thus the complete list of libraries might need to be adjusted depending on
# your installation.
#

# Default compiler for C and C++ (CPP is normally the C preprocessor)
CC = gcc
CXX = g++

BINDIR = ./bin
LIBDIR = ./lib
OBJDIR = ./obj
LIB3DDIR = ./lib3d
# QTDIR should come from the environment
ifndef QTDIR
$(error The QTDIR envirnoment variable is not defined. Define it as the path \
	to the QT installation directory)
endif
MOC = $(QTDIR)/bin/moc

# Extra compiler flags (optimization, profiling, debug, etc.)
XTRA_CFLAGS = -g -O2 -ansi -march=i686
XTRA_CXXFLAGS = -g -O2 -ansi
XTRA_LDFLAGS = -g

# Add profiling flags if requested
ifdef PROFILE
XTRA_CFLAGS += -pg
XTRA_CXXFLAGS += -pg
XTRA_LDFLAGS += -pg
ifeq ($(PROFILE),full)
XTRA_LDFLAGS += -static
endif
endif

# Source files and executable name
VIEWER_EXE := $(BINDIR)/viewer
VIEWER_C_SRCS := $(wildcard *.c)
VIEWER_CXX_SRCS := $(filter-out moc_%.cpp,$(wildcard *.cpp))
VIEWER_MOC_SRCS := RawWidget.h ScreenWidget.h InitWidget.h
LIB3D_C_SRCS = 3dmodel_io.c normals.c geomutils.c

# Compiler and linker flags
INCFLAGS = -I$(LIB3DDIR)/include -I.
QTINCFLAGS = -I$(QTDIR)/include
GLINCFLAGS = -I/usr/X11R6/include

# Libraries and search path for final linking
ifeq ($(PROFILE),full)
LDLIBS = -lqt -lGL -lGLU -lXmu -lXext -lSM -lICE -lXft -lpng -ljpeg -lmng \
	-lXi -ldl -lXt -lz -lfreetype -lXrender -lX11 -lm_p -lc_p
else
LDLIBS = -lqt -lGL -lGLU -lXmu -lXext -lX11 -lm
endif
LOADLIBES = -L$(QTDIR)/lib -L/usr/X11R6/lib
LDFLAGS =

# C and C++ warning flags
WARN_CFLAGS = -pedantic -Wall -W -Winline -Wmissing-prototypes \
	-Wstrict-prototypes -Wnested-externs -Wshadow -Waggregate-return
WARN_CXXFLAGS = -pedantic -Wall -W -Wmissing-prototypes

# Preprocessor flags
CPPFLAGS = $(INCFLAGS) -D_METRO

# Construct basic compiler flags
CFLAGS = $(WARN_CFLAGS) $(XTRA_CFLAGS)
CXXFLAGS = $(WARN_CXXFLAGS) $(XTRA_CXXFLAGS)
LDFLAGS = $(XTRA_LDFLAGS)

# Automatically derived file names
MOC_CXX_SRCS = $(addprefix moc_,$(VIEWER_MOC_SRCS:.h=.cpp))
VIEWER_OBJS = $(addprefix $(OBJDIR)/, $(VIEWER_C_SRCS:.c=.o) \
	$(VIEWER_CXX_SRCS:.cpp=.o) $(MOC_CXX_SRCS:.cpp=.o))
LIB3D_OBJS = $(addprefix $(OBJDIR)/,$(LIB3D_C_SRCS:.c=.o))
LIB3D_SLIB = $(addprefix $(LIBDIR)/,lib3d.a)

#
# Targets
#

# Main targets
default: $(VIEWER_EXE)

all: dirs $(VIEWER_EXE)

clean: 
	-rm -f *.d $(OBJDIR)/*.o $(BINDIR)/* $(LIBDIR)/*

# Executable
$(VIEWER_EXE): $(VIEWER_OBJS) $(LIB3D_SLIB)
	$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

# LIB3D static library (only what we need of lib3d)
# GNU make automatic rule for archives will be used here
$(LIB3D_SLIB): $(LIB3D_SLIB)($(LIB3D_OBJS))

# QT/OpenGL GUI (C++)
$(OBJDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(QTINCFLAGS) $(GLINCFLAGS) $< -o $@

# Produce QT moc sources
moc_%.cpp: %.h
	$(MOC) $< -o $@

# Error computing functions
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# lib3d sources
$(LIB3D_OBJS): $(OBJDIR)/%.o : $(LIB3DDIR)/src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

#
# Automatic dependency
#

ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
include $(VIEWER_C_SRCS:.c=.d) $(LIB3D_C_SRCS:.c=.d) $(VIEWER_CXX_SRCS:.cpp=.d)
endif
# Regexp escaped version of $(OBJDIR)/
OBJDIRRE := $(shell echo $(OBJDIR)/ | sed 's/\./\\\./g;s/\//\\\//g;')

$(VIEWER_C_SRCS:.c=.d): %.d: %.c
	set -e; $(CC) -M $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/$(OBJDIRRE)\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@
$(LIB3D_C_SRCS:.c=.d): %.d : $(LIB3DDIR)/src/%.c
	set -e; $(CC) -M $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/$(OBJDIRRE)\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@
$(VIEWER_CXX_SRCS:.cpp=.d): %.d : %.cpp
	set -e; $(CXX) -M $(CPPFLAGS) $(QTINCFLAGS) $(GLINCFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/$(OBJDIRRE)\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

#
# Directories
#
dirs : libdir bindir objdir

libdir : 
	-[ -d $(LIBDIR) ] || mkdir $(LIBDIR)

bindir : 
	-[ -d $(BINDIR) ] || mkdir $(BINDIR)

objdir :
	-[ -d $(OBJDIR) ] || mkdir $(OBJDIR)

# Targets which are not real files
.PHONY: default all dirs clean libdir bindir objdir
