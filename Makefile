# $Id: Makefile,v 1.53 2002/08/30 09:18:39 aspert Exp $



#
#  Copyright (C) 2001-2002 EPFL (Swiss Federal Institute of Technology,
#  Lausanne) This program is free software; you can redistribute it
#  and/or modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA.
#
#  In addition, as a special exception, EPFL gives permission to link
#  the code of this program with the Qt non-commercial edition library
#  (or with modified versions of Qt non-commercial edition that use the
#  same license as Qt non-commercial edition), and distribute linked
#  combinations including the two.  You must obey the GNU General
#  Public License in all respects for all of the code used other than
#  Qt non-commercial edition.  If you modify this file, you may extend
#  this exception to your version of the file, but you are not
#  obligated to do so.  If you do not wish to do so, delete this
#  exception statement from your version.
#
#  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
#
#  Web site : http://mesh.epfl.ch
#
#  Reference :
#   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
#   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002,
#   vol. I, pp. 705-708, available on http://mesh.epfl.ch
#
#


# If the MPATROL library is defined to a non-empty value, the resulting
# executable is linked to the mpatrol library for memory checking. If the
# value of the variable contains the 'c-mem-check' string (without quotes)
# all C source files will be compiled with full pointer dereferencing checking.
# Likewise, if the variable contains the 'cxx-mem-check' string, all C++ source
# files will be compiled with full pointer dereferencing checking. These two
# last options require GCC and will significantly slow down the execution.

# If the make variable PROFILE is defined to a non-empty value, profiling
# flags are automatically added. If the value of the variable is 'full'
# (without the quotes) the executable is linked with the profiling versions
# of the standard C library.
# Note that 'full' requires the creation of a fully static executable, and
# thus the complete list of libraries might need to be adjusted depending on
# your installation.


# Mesh's version: is the string defined as version in mesh.cpp
# This is an ugly sed command, but it ensures that the version number is
# always in sync with the one in mesh.cpp.
MESHVER := $(shell sed '/char *\* *version *= *"[^"]*" *; *$$/ { s/[^"]*"\([^"]*\)".*/\1/; p; } ; d' mesh.cpp )

# Autodetect platform
OS := $(shell uname -s)
ARCH := $(shell uname -m)
CPU = P3 #default -> Pentium III
ifeq ($(ARCH),i686)
# Let's try to autodetect the CPU ...
TMP1 := $(shell grep "model name" /proc/cpuinfo | uniq | cut -d: -f2 | awk '{print $$1$$2}')
# Just for Pentium 4 and Athlon XP
TMP2 := $(shell grep "model name" /proc/cpuinfo | uniq | cut -d: -f2 | awk '{print $$2$$3}')

ifeq ($(TMP1),PentiumII) # Pentium II
CPU = P2
endif
ifeq ($(TMP1),PentiumIII) # Pentium III
CPU = P3
endif
ifeq ($(TMP2),Pentium(R)4) # Pentium 4
CPU = P4
endif

ifeq ($(TMP1),AMDAthlon(tm)) # Athlon
CPU = ATHLON
endif
ifeq ($(CPU), ATHLON) # test for Athlon XPs
ifeq ($(TMP2),Athlon(tm)XP)
CPU = ATHLONXP
endif
endif

endif # ifeq($(ARCH) i686)

# If you want to override platform detection 
# (in order to compile for another one for instance), set the CPU var. 
# to the desired value below
# Possible values : P2, P3, P4, ATHLON and ATHLONXP
# CPU = P4


# If OS is IRIX64 make it IRIX since it's the same for us
ifeq ($(OS),IRIX64)
OS := IRIX
endif

# Default compiler for C and C++ 
ifeq ($(OS),Linux)
CC = gcc
CXX = g++
endif
ifeq ($(OS),IRIX)
CC = cc
CXX = CC
endif
ifeq ($(OS),SunOS)
CC = cc
CXX = CC
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

# Autodetect PGCC (Portland Group compiler)
CC_IS_PGCC := $(findstring pgcc,$(shell $(CC) -V 2>&1))

# Autodetect GCC
ifneq ($(CC_IS_PGCC),pgcc)
CC_IS_GCC := $(findstring gcc,$(shell $(CC) -v 2>&1))
endif
CXX_IS_GCC := $(findstring gcc,$(shell $(CXX) -v 2>&1))

# Autodetect ICC
CC_IS_ICC := $(findstring Intel,$(shell $(CC) -V 2>&1))

# Autodetect MipsPro compilers
CC_IS_MP := $(findstring MIPSpro,$(shell $(CC) -version 2>&1))
CXX_IS_MP := $(findstring MIPSpro,$(shell $(CXX) -version 2>&1))

# Autodetect Solaris WorkShop compilers
CC_IS_WS := $(findstring WorkShop,$(shell $(CC) -V 2>&1))
CXX_IS_WS := $(findstring WorkShop,$(shell $(CXX) -V 2>&1))

# Extra compiler flags (optimization, profiling, debug, etc.)
ifeq ($(CC_IS_WS)-$(OS),WorkShop-SunOS)
XTRA_CFLAGS = -xO2 # equivalent to '-O2'
XTRA_CXXFLAGS = -xO2
DEPFLAG = -xM 
else
XTRA_CFLAGS = -O2
XTRA_CXXFLAGS = -O2
DEPFLAG = -M
endif
XTRA_CPPFLAGS = -DNDEBUG
XTRA_LDFLAGS =


# Derive compiler specific flags
ifeq ($(CC_IS_GCC)-$(OS)-$(ARCH),gcc-Linux-i686)
XTRA_CFLAGS += -march=i686 -malign-double 
endif
ifeq ($(CC_IS_GCC),gcc)
C_PROF_OPT = -pg
XTRA_CFLAGS += -ansi -g -pipe
WARN_CFLAGS = -pedantic -Wall -W -Winline -Wmissing-prototypes \
        -Wstrict-prototypes -Wnested-externs -Wshadow -Waggregate-return
# Following options might produce incorrect behaviour if code
# is modified (only ANSI C aliasing allowed, and no math error checking)
XTRA_CFLAGS += -fstrict-aliasing -fno-math-errno
endif

ifeq ($(CC_IS_ICC),Intel)
C_PROF_OPT = -p
# Target processor: i: Pentium Pro/II, M: MMX, K: streaming SIMD - SSE,
# W: Pentium IV (W implies iMK and K implies iM, but M does not imply i)
ifeq ($(CPU),P2)
XTRA_CFLAGS += -ansi -g -tpp6 -ip
XTRA_CFLAGS += -xiM
endif
ifeq ($(CPU),P3)
XTRA_CFLAGS += -ansi -g -tpp6 -ip
XTRA_CFLAGS += -xK
endif
ifeq ($(CPU),P4)
XTRA_CFLAGS += -ansi -g -tpp7 -ip
XTRA_CFLAGS += -xW
endif
ifeq ($(CPU),ATHLON)
XTRA_CFLAGS += -ansi -g -tpp6 -ip
XTRA_CFLAGS += -xK
endif
ifeq ($(CPU),ATHLONXP)
XTRA_CFLAGS += -ansi -g -tpp7 -ip
XTRA_CFLAGS += -xW
endif
endif

ifeq ($(CC_IS_PGCC)-$(OS),pgcc-Linux)
#XTRA_CFLAGS += -Mnodalign
XTRA_CFLAGS += -Mvect -Munroll -Mcache_align -Mvect=smallvect:3 \
	-Mnoframe -Mnoreentrant
# PGCC does not obey the __inline flag, so force inlining
XTRA_CFLAGS += -Minline=dist_sqr_pt_cell -Minline=size:50,levels=3

ifeq ($(CPU), P4)
# Specify target processor:  p7 (Pentium IV)
XTRA_CFLAGS += -tp p7
# Enable vectorized floating-point (requires Pentium III/IV or AthlonXP)
XTRA_CFLAGS += -Mvect=sse
# Enable prefetch (requires Pentium III/IV, Athlon or AthlonXP)
XTRA_CFLAGS += -Mvect=prefetch
endif

ifeq ($(CPU), ATHLONXP)
# Specify target processor:  athlonxp (AMD Athlon XP)
XTRA_CFLAGS += -tp athlonxp
# Enable vectorized floating-point (requires Pentium III/IV or AthlonXP)
XTRA_CFLAGS += -Mvect=sse
# Enable prefetch (requires Pentium III/IV, Athlon or AthlonXP)
XTRA_CFLAGS += -Mvect=prefetch
endif

ifeq ($(CPU), ATHLON)
# Specify target processor:  athlon (AMD Athlon)
XTRA_CFLAGS += -tp athlon
# Enable prefetch (requires Pentium III/IV, Athlon or AthlonXP)
XTRA_CFLAGS += -Mvect=prefetch
endif

ifeq ($(CPU), P3)
# Specify target processor: p6 (Pentium Pro/II/III)
XTRA_CFLAGS += -tp p6
# Enable vectorized floating-point (requires Pentium III/IV or AthlonXP)
XTRA_CFLAGS += -Mvect=sse
# Enable prefetch (requires Pentium III/IV, Athlon or AthlonXP)
XTRA_CFLAGS += -Mvect=prefetch
endif

ifeq ($(CPU), P2)
# Specify target processor: p6 (Pentium Pro/II/III)
XTRA_CFLAGS += -tp p6
endif

# Need these so that g++ can link PGCC compiled objects
XTRA_LDFLAGS += -L$(PGI)/linux86/lib
XTRA_LDLIBS += -lpgc
endif
ifeq ($(CC_IS_MP)-$(OS),MIPSpro-IRIX)
C_PROF_OPT = -fbgen
XTRA_CFLAGS += -ansi -IPA -g3
XTRA_LDFLAGS += -IPA
endif
ifeq ($(CC_IS_WS)-$(OS),WorkShop-SunOS)
C_PROF_OPT = -xpg 
XTRA_CFLAGS = -Xc -g -xlibmil -xCC # -Xc equivalent to -ansi in other compilers
endif

# C++ compiler options
ifeq ($(CXX_IS_GCC),gcc)
CXX_PROF_OPT = -pg
XTRA_CXXFLAGS += -ansi -g -pipe
WARN_CXXFLAGS = -pedantic -Wall -W -Wmissing-prototypes
endif
ifeq ($(CXX_IS_MP)-$(OS),MIPSpro-IRIX)
CXX_PROF_OPT = -fbgen
XTRA_CXXFLAGS += -ansi -g3
endif
ifeq ($(CXX_IS_WS)-$(OS),WorkShop-SunOS)
CXX_PROF_OPT = -xpg
XTRA_CXXFLAGS = -g -xlibmil
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
ifeq ($(OS),SunOS)
$(error Need to add list of mpatrol libraries for Solaris)
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
LIB3D_C_SRCS = geomutils.c model_in.c model_in_raw.c model_in_smf.c \
	model_in_ply.c model_in_vrml_iv.c

# Files for distribution
MISC_FILES = Makefile Mesh.dsp Mesh.dsw meshIcon.xpm README COPYING AUTHORS \
	CHANGELOG
LIB3D_INCLUDES = 3dmodel.h geomutils.h model_in.h model_in_ply.h types.h
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
	-rm -f $(LIB3D_OBJS) $(addprefix $(OBJDIR)/, $(MESH_C_SRCS:.c=.o))

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
	gzip -9 -c Mesh-$(MESHVER).tar > Mesh-$(MESHVER).tar.gz && \
	bzip2 Mesh-$(MESHVER).tar && \
	rm -rf Mesh-$(MESHVER) || \
	rm -f Mesh-$(MESHVER).tar Mesh-$(MESHVER).tar.gz Mesh-$(MESHVER).zip \
	Mesh-$(MESHVER).tar.bz2

#
# Automatic dependency
#

ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
ifneq ($(findstring dist,$(MAKECMDGOALS)), dist)
include $(MESH_C_SRCS:.c=.d) $(LIB3D_C_SRCS:.c=.d) $(MESH_CXX_SRCS:.cpp=.d)
endif
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
