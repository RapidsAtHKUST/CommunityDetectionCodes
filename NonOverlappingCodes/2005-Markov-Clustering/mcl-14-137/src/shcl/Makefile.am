## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.49 2007-05-26 20:14:26 flux Exp $

include $(top_srcdir)/include/include.am

clmformat.zmm.h: clmformat.zmm
	../../scripts/cfy clmformat.zmm clmformat.zmm.h defs_zmm

clmps.defs.h: clmps.defs
	../../scripts/cfy clmps.defs clmps.defs.h defs_ps

clmformat.c: clmformat.zmm.h

bin_PROGRAMS = clmformat clm
noinst_PROGRAMS = clmdag clmframe

clmformat_SOURCES = clmformat.c report.c
clmframe_SOURCES = clmframe.c report.c
clmdag_SOURCES = clmdag.c report.c

clm_SOURCES = clm.c clminfo2.c clminfo.c clmimac.c clmadjust.c clmresidue.c clmclose.c clmmate.c \
					clmmeet.c clmps.c clmdist.c clmorder.c clmoptics.c report.c

bin_SCRIPTS = mclpipeline
noinst_SCRIPTS = mclgraga mclgrep

EXTRA_DIST = clmorder.h clmdist.h clminfo2.h clminfo.h clmclose.h clmmate.h clmmeet.h clm.h \
					clmadjust.h clmoptics.h clmresidue.h clmimac.h clmps.h clmps.defs.h\
					report.h report.c \
					$(bin_SCRIPTS) \
               mclgraga mclgrep clmformat.zmm clmformat.zmm.h \
					clmps.defs clmps.defs.h

