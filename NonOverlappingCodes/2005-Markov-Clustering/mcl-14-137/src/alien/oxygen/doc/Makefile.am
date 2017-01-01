## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.8 2004/08/15 09:09:45 flux Exp $

TROFF =  groff
COL   =  col
ZOEM  =  zoem

docrepository = ../../../../doc
zmmval = '"mcx-zmm"=../../../../doc/mcx.zmm'

## perhaps could better run zoem as installed on system, so just
## say
##  ZOEM = zoem


## tidy is a HTML syntax checker and reformatter, available from
## http://www.w3.org/People/Raggett/tidy/ , by Dave Raggett <dsr@w3.org>
TIDY = tidy

docdir = $(datadir)/doc/$(PACKAGE)

## zoemtut.html is "on hold": do not install; zoem is not yet ready
## for prime time
doc_DATA = mcxdeblast.html mcxdeblast.ps mclblastline.html mclblastline.ps

MAINTAINERCLEANFILES = mcxdeblast.1 mclblastline.1 $(doc_DATA)
EXTRA_DIST = mcxdeblast.azm mclblastline.azm \
   $(MAINTAINERCLEANFILES)

man_MANS = mcxdeblast.1 mclblastline.1


mclblastline: mclblastline.html mclblastline.1 mclblastline.ps

mclblastline.html: mclblastline.azm
	$(ZOEM) -s $(zmmval) -i mclblastline -d html -o mclblastline.html
	$(ZOEM) -s $(zmmval) -i mclblastline -d html -o mclblastline.html
	cp mclblastline.html $(docrepository)
	- $(TIDY) -e mclblastline.html

mclblastline.1: mclblastline.azm
	$(ZOEM) -s $(zmmval) -i mclblastline -d roff -o  mclblastline.1
	$(ZOEM) -s $(zmmval) -i mclblastline -d roff -o  mclblastline.1

mclblastline.ps: mclblastline.1
	$(TROFF) -man mclblastline.1 > mclblastline.ps
	cp mclblastline.ps $(docrepository)


mcxdeblast: mcxdeblast.html mcxdeblast.1 mcxdeblast.ps

mcxdeblast.html: mcxdeblast.azm
	$(ZOEM) -s $(zmmval) -i mcxdeblast -d html -o mcxdeblast.html
	$(ZOEM) -s $(zmmval) -i mcxdeblast -d html -o mcxdeblast.html
	cp mcxdeblast.html $(docrepository)
	- $(TIDY) -e mcxdeblast.html

mcxdeblast.1: mcxdeblast.azm
	$(ZOEM) -s $(zmmval) -i mcxdeblast -d roff -o  mcxdeblast.1
	$(ZOEM) -s $(zmmval) -i mcxdeblast -d roff -o  mcxdeblast.1

mcxdeblast.ps: mcxdeblast.1
	$(TROFF) -man mcxdeblast.1 > mcxdeblast.ps
	cp mcxdeblast.ps $(docrepository)

