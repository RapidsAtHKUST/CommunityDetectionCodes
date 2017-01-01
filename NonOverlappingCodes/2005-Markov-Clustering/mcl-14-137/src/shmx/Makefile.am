## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.39 2007-05-25 19:43:28 flux Exp $

include $(top_srcdir)/include/include.am

bin_PROGRAMS = mcx mcxsubs mcxmap mcxassemble mcxarray \
						mcxdump mcxload mcxrand
noinst_PROGRAMS = mcxtest2 mcxtest mcxminusmeet mcxmm mcxmetric

EXTRA_DIST = fake mcx.h mcxconvert.h mcxminusmeet.c mcxquery.h mcxdiameter.h mcxclcf.h mcxerdos.h mcxcollect.h mcxtab.h mcxfp.h mcxalter.h

mcxassemble_SOURCES = mcxassemble.c
mcxsubs_SOURCES = mcxsubs.c
mcxmap_SOURCES = mcxmap.c
mcxdump_SOURCES = mcxdump.c
mcxtest_SOURCES = mcxtest.c
mcxtest2_SOURCES = mcxtest2.c
mcxarray_SOURCES = mcxarray.c
mcxload_SOURCES = mcxload.c
mcxrand_SOURCES = mcxrand.c
mcxmm_SOURCES = mcxmm.c
mcxmetric_SOURCES = mcxmetric.c
mcxminusmeet_SOURCES = mcxminusmeet.c

mcx_SOURCES = mcx.c mcxconvert.c mcxquery.c mcxdiameter.c mcxclcf.c mcxerdos.c mcxcollect.c mcxtab.c mcxfp.c mcxalter.c

# leave: assemble erdos array rand subs
# assimilate: map dump test array load minusmeet


