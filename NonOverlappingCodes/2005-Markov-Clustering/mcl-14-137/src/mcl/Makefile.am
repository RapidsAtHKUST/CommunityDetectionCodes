## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.17 2007-03-30 21:48:58 flux Exp $

## bin_PROGRAMS = mcl
## mcl_SOURCES = mcl.c mcl.h
## mcl_LDADD = @LIBOBJS@

include $(top_srcdir)/include/include.am

noinst_LIBRARIES = libmcl.a
libmcl_a_SOURCES = shadow.c inflate.c alg.c expand.c dpsd.c interpret.c proc.c procinit.c transform.c

EXTRA_DIST = shadow.h inflate.h alg.h expand.h dpsd.h interpret.h proc.h transform.h \
  procinit.h

