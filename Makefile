# $Id: Makefile,v 1.34 2002/02/27 09:54:41 aspert Exp $

#
# If the make variable PROFILE is defined to a non-empty value, profiling
# flags are automatically added. If the value of the variable is 'full'
# (without the quotes) the executable is linked with the profiling versions
# of the standard C library.
# Note that 'full' requires the creation of a fully static executable, and
# thus the complete list of libraries might need to be adjusted depending on
# your installation.
#

#
# If the MPATROL library is defined to a non-empty value, the resulting
# executable is linked to the mpatrol library for memory checking. If the
# value of the variable contains the 'c-mem-check' string (without quotes)
# all C source files will be compiled with full pointer dereferencing checking.
# Likewise, if the variable contains the 'cxx-mem-check' string, all C++ source
# files will be compiled with full pointer dereferencing checking. These two
# last options require GCC and will significantly slow down the execution.
#

# Mesh's version: is the string defined as version in mesh.cpp
# This is an ugly sed command, but it ensures that the version number is
# always in sync with the one in mesh.cpp.
MESHVER := $(shell sed '/char *\* *version *= *"[^"]*" *; *$$/ { s/[^"]*"\([^"]*\)".*/\1/; p; } ; d' mesh.cpp )

# Autodetect platform
OS := $(shell uname -s)
ARCH := $(shell uname -m)

# If OS is IRIX64 make it IRIX since it's the same for us
ifeq ($(OS),IRIX64)
OS := IRIX
endif

# Default compiler for C and C++ (CPP is normally the C preprocessor)
ifeq ($(OS),Linux)
CC = gcc
CXX = g++
DEPFLAG = -M
endif
ifeq ($(OS),IRIX)
CC = cc
CXX = CC
DEPFLAG = -M
endif
ifeq ($(OS),SunOS)
CC = cc
CXX = CC
DEPFLAG = -xM
endif
# Default directories
BINDIR = ./bin
LIBDIR = ./lib
OBJDIR = ./obj
DISTDIR = ./dist
LIB3DDIR = ./lib3d
# QTDIR should come from the environment
ifndef QTDIR
$(error The QTDIR environment variable is not defined. Define it as the path \
	to the QT installation directory)
endif

# Auxiliary executables
MOC = $(QTDIR)/bin/moc

# Autodetect GCC and/or ICC
CC_IS_GCC := $(findstring gcc,$(shell $(CC) -v 2>&1))
CXX_IS_GCC := $(findstring gcc,$(shell $(CXX) -v 2>&1))
CC_IS_ICC := $(findstring Intel,$(shell $(CC) -V 2>&1))

# Extra compiler flags (optimization, profiling, debug, etc.)
ifneq ($(OS),SunOS)
XTRA_CFLAGS = -O2 -ansi
XTRA_CXXFLAGS = -O2 -ansi
XTRA_CPPFLAGS = -DNDEBUG
XTRA_LDFLAGS =
else
XTRA_CFLAGS = -xO2
XTRA_CXXFLAGS = -xO2
XTRA_CPPFLAGS = -DNDEBUG
XTRA_LDFLAGS =
endif
ifeq ($(OS),Linux)
# Need -D_GNU_SOURCE for getting non-standard unlocked stdio functions
XTRA_CPPFLAGS +=  -D_GNU_SOURCE
endif

# Derive compiler specific flags
ifeq ($(CC_IS_GCC)-$(OS)-$(ARCH),gcc-Linux-i686)
XTRA_CFLAGS += -march=i686
endif
ifeq ($(CC_IS_GCC),gcc)
C_PROF_OPT = -pg
XTRA_CFLAGS += -g -pipe
WARN_CFLAGS = -pedantic -Wall -W -Winline -Wmissing-prototypes \
        -Wstrict-prototypes -Wnested-externs -Wshadow -Waggregate-return
# Following options might produce incorrect behaviour if code
# is modified (only ANSI C aliasing allowed, and no math error checking)
XTRA_CFLAGS += -fstrict-aliasing -fno-math-errno
endif
ifeq ($(CC_IS_ICC),Intel)
C_PROF_OPT = -p
XTRA_CFLAGS += -g -tpp6 -ip
endif
ifeq ($(CC_IS_ICC)-$(OS)-$(ARCH),Intel-Linux-i686)
XTRA_CFLAGS += -xiM
endif
ifeq ($(CC_IS_GCC)$(OS),IRIX)
C_PROF_OPT = -fbgen
XTRA_CFLAGS += -IPA -g3
XTRA_LDFLAGS += -IPA
endif
ifeq ($(CXX_IS_GCC),gcc)
CXX_PROF_OPT = -pg
XTRA_CXXFLAGS += -g -pipe
WARN_CXXFLAGS = -pedantic -Wall -W -Wmissing-prototypes
endif
ifeq ($(CXX_IS_GCC)$(OS),IRIX)
CXX_PROF_OPT = -fbgen
XTRA_CXXFLAGS += -g3
endif

# Add profiling flags if requested
ifdef PROFILE
XTRA_CFLAGS += $(C_PROF_OPT)
XTRA_CXXFLAGS += $(CXX_PROF_OPT)
XTRA_LDFLAGS += $(CXX_PROF_OPT)
ifeq ($(PROFILE)-$(OS),full-Linux)
XTRA_LDFLAGS += -static
endif
endif

# Add MPATROL flags if requested
ifdef MPATROL
ifeq ($(OS),Linux)
XTRA_LDLIBS += -lmpatrol -lbfd -liberty
endif
ifeq ($(OS),IRIX)
$(error Need to add list of mpatrol libraries for IRIX)
endif
ifeq ($(findstring c-mem-check,$(MPATROL)),c-mem-check)
ifeq ($(CC_IS_GCC),gcc)
XTRA_CFLAGS += -fcheck-memory-usage
else
$(error c-mem-check mpatrol option only supported with GCC C compiler)
endif
endif
ifeq ($(findstring cxx-mem-check,$(MPATROL)),cxx-mem-check)
ifeq ($(CXX_IS_GCC),gcc)
XTRA_CXXFLAGS += -fcheck-memory-usage
else
$(error cxx-mem-check mpatrol option only supported with GCC C++ compiler)
endif
endif
endif

# Source files and executable name
MESH_EXE := $(BINDIR)/mesh
MESH_C_SRCS := $(wildcard *.c)
MESH_CXX_SRCS := $(filter-out moc_%.cpp,$(wildcard *.cpp))
MESH_MOC_SRCS := RawWidget.h ScreenWidget.h InitWidget.h ColorMapWidget.h
LIB3D_C_SRCS = geomutils.c model_in.c

# Files for distribution
MISC_FILES = Makefile Mesh.dsp Mesh.dsw meshIcon.xpm
LIB3D_INCLUDES = 3dmodel.h geomutils.h model_in.h
MESH_INCLUDES := $(wildcard *.h)

# Compiler and linker flags
INCFLAGS = -I$(LIB3DDIR)/include -I.
QTINCFLAGS = -I$(QTDIR)/include
GLINCFLAGS = -I/usr/X11R6/include

# Libraries and search path for final linking
ifeq ($(PROFILE)-$(OS),full-Linux)
LDLIBS = -lqt -lGL -lGLU -lXmu -lXext -lSM -lICE -lXft -lpng -ljpeg -lmng \
	-lXi -ldl -lXt -lz -lfreetype -lXrender -lX11
XTRA_LDLIBS += -lm_p -lc_p
else
LDLIBS = -lqt -lGL -lGLU -lXmu -lXext -lX11 -lm
endif
LOADLIBES = -L$(QTDIR)/lib -L/usr/X11R6/lib
LDFLAGS =

# Preprocessor flags
CPPFLAGS = $(INCFLAGS) $(XTRA_CPPFLAGS)

# Construct basic compiler flags
CFLAGS = $(WARN_CFLAGS) $(XTRA_CFLAGS)
CXXFLAGS = $(WARN_CXXFLAGS) $(XTRA_CXXFLAGS)
LDFLAGS = $(XTRA_LDFLAGS)
LDLIBS += $(XTRA_LDLIBS)

# Automatically derived file names
MOC_CXX_SRCS = $(addprefix moc_,$(MESH_MOC_SRCS:.h=.cpp))
MESH_OBJS = $(addprefix $(OBJDIR)/, $(MESH_C_SRCS:.c=.o) \
	$(MESH_CXX_SRCS:.cpp=.o) $(MOC_CXX_SRCS:.cpp=.o))
LIB3D_OBJS = $(addprefix $(OBJDIR)/,$(LIB3D_C_SRCS:.c=.o))
LIB3D_SLIB = $(addprefix $(LIBDIR)/,lib3d.a)

#
# Targets
#

# NOTE: moc generated C++ files are temporary files and automatically removed
# by GNU make after compile, so we make sure they are not present as byproduct
# of another build on same directory (e.g., Visual C++)

# Main targets
default: all

all: dirs $(MESH_EXE)

clean: clean_moc clean_dist
	-rm -f *.d $(OBJDIR)/*.o $(OBJDIR)/*.il $(BINDIR)/* $(LIBDIR)/*

clean_moc:
	-rm -f moc_*.cpp

clean_dist:
	-rm -rf $(DISTDIR)/*

clean-c:
	-rm $(LIB3D_OBJS) $(addprefix $(OBJDIR)/, $(MESH_C_SRCS:.c=.o))

# Executable
$(MESH_EXE): $(MESH_OBJS) $(LIB3D_SLIB)
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
# Distribution
#

dist: distdir
	rm -rf $(DISTDIR)/Mesh-$(MESHVER) $(DISTDIR)/Mesh-$(MESHVER).tar.gz && \
	mkdir -p $(DISTDIR)/Mesh-$(MESHVER) \
		$(DISTDIR)/Mesh-$(MESHVER)/$(LIB3DDIR)/{src,include} && \
	cp $(MISC_FILES) $(MESH_C_SRCS) $(MESH_CXX_SRCS) $(MESH_INCLUDES) \
		$(DISTDIR)/Mesh-$(MESHVER) && \
	cp $(addprefix $(LIB3DDIR)/include/,$(LIB3D_INCLUDES)) \
		$(DISTDIR)/Mesh-$(MESHVER)/$(LIB3DDIR)/include && \
	cp $(addprefix $(LIB3DDIR)/src/,$(LIB3D_C_SRCS)) \
		$(DISTDIR)/Mesh-$(MESHVER)/$(LIB3DDIR)/src
	cd $(DISTDIR) && \
	zip -9 -r Mesh-$(MESHVER).zip Mesh-$(MESHVER) && \
	tar cvf Mesh-$(MESHVER).tar Mesh-$(MESHVER) && \
	gzip -9 Mesh-$(MESHVER).tar && \
	rm -rf Mesh-$(MESHVER) || \
	rm -f Mesh-$(MESHVER).tar Mesh-$(MESHVER).tar.gz Mesh-$(MESHVER).zip

#
# Automatic dependency
#

ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
include $(MESH_C_SRCS:.c=.d) $(LIB3D_C_SRCS:.c=.d) $(MESH_CXX_SRCS:.cpp=.d)
endif
# Regexp escaped version of $(OBJDIR)/
OBJDIRRE := $(shell echo $(OBJDIR)/ | sed 's/\./\\\./g;s/\//\\\//g;')

$(MESH_C_SRCS:.c=.d): %.d: %.c
	set -e; $(CC) $(DEPFLAG) $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/$(OBJDIRRE)\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@
$(LIB3D_C_SRCS:.c=.d): %.d : $(LIB3DDIR)/src/%.c
	set -e; $(CC) $(DEPFLAG) $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/$(OBJDIRRE)\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@
$(MESH_CXX_SRCS:.cpp=.d): %.d : %.cpp
	set -e; $(CXX) $(DEPFLAG) $(CPPFLAGS) $(QTINCFLAGS) $(GLINCFLAGS) $< \
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

distdir :
	-[ -d $(DISTDIR) ] || mkdir $(DISTDIR)

# Targets which are not real files
.PHONY: default all dirs clean libdir bindir objdir distdir
