## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.29 2006-01-23 23:07:49 flux Exp $

SUBDIRS = . util img graphs doc src include scripts testing

VERSION = @VERSION@

AM_CPPFLAGS = -DMAN_PATH=\"$(mandir)\"

configure.ac: configure.ac.in VERSION
	./setversion

EXTRA_DIST = \
   LICENSE VERSION README COPYING THANKS INSTALL AUTHORS \
   bootstrap \
   configure.ac.in \
   setversion

