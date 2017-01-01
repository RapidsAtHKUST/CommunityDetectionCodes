## Process this file with automake to produce Makefile.in
## $Id: Makefile.am,v 1.16 2007-05-18 11:02:25 flux Exp $

TROFF =  groff
COL   =  col
ZOEM  =  zoem

## tidy is a HTML syntax checker and reformatter, available from
## http://www.w3.org/People/Raggett/tidy/ , by Dave Raggett <dsr@w3.org>
TIDY = tidy

docdir = $(datadir)/doc/$(PACKAGE)

# mclfamily.html mclfamily.ps

doc_DATA_dep = mcl.html mcl.ps mcl.txt \
 mclcm.html mclcm.ps mclcm.txt \
 mclfaq.html mclfaq.ps mclfaq.txt \
 mclfamily.html mclfamily.ps \
 mcx.html mcx.ps \
 mcxi.html mcxi.ps \
 clm.html clm.ps \
 mcxconvert.html mcxconvert.ps \
 mcxarray.html mcxarray.ps \
 mcxdump.html mcxdump.ps \
 mcxdiameter.html mcxdiameter.ps \
 mcxerdos.html mcxerdos.ps \
 mcxalter.html mcxalter.ps \
 mcxclcf.html mcxclcf.ps \
 mcxctty.html mcxctty.ps \
 mcxrand.html mcxrand.ps \
 mcxload.html mcxload.ps \
 mcxsubs.html mcxsubs.ps \
 mcxmap.html mcxmap.ps \
 mcxquery.html mcxquery.ps \
 mcxassemble.html mcxassemble.ps \
 mclpipeline.html mclpipeline.ps \
 index.html mclindex.ps \
 clmimac.html clmimac.ps \
 clmoptics.html clmoptics.ps \
 clmclose.html clmclose.ps \
 clmresidue.html clmresidue.ps \
 clmdist.html clmdist.ps \
 clmvol.html clmvol.ps \
 clminfo.html clminfo.ps \
 clminfo2.html clminfo2.ps \
 clmmate.html clmmate.ps \
 clmmeet.html clmmeet.ps \
 clmformat.html clmformat.ps \
 clmorder.html clmorder.ps \
 minimcl \
 mcxio.html mcxio.ps \
 clmprotocols.html clmprotocols.ps \
 clmprotocols2.html clmprotocols2.ps \
 tingea.log.html tingea.log.ps

doc_DATA = $(doc_DATA_dep) mclmcl.txt

man_MANS = mcl.1 mclcm.1 clm.1 mclindex.7 mclfamily.7 mclfaq.7 mcx.1 mcxi.1 mcxio.5 clmprotocols2.5 clmprotocols.5 \
   clmresidue.1 clmclose.1 clmoptics.1 clmimac.1 clmmate.1 clmmeet.1 clmformat.1 \
   clmvol.1 clmdist.1 clminfo2.1 clminfo.1 clmorder.1 \
   mcxconvert.1 mcxassemble.1 mcxmap.1 mcxsubs.1 mcxload.1 mcxdump.1 mcxarray.1 mcxrand.1 \
	mcxalter.1 mcxerdos.1 mcxdiameter.1 mcxctty.1 mcxclcf.1 mcxquery.1 \
   mclpipeline.1 \
	tingea.log.7



MAINTAINERCLEANFILES = $(man_MANS) $(doc_DATA_dep)

ALIENFILES = mcxdeblast.html mcxdeblast.ps mclblastline.html mclblastline.ps
## This is quite hackish (though not deep).
## In the alien files, these are copied to this directory.
## This is needed so that hyperlinks work correctly when this
## documentation is accessed directly from the tarball, rather than
## after an installation.
## So this introduces the dependency that alien documentation must
## be processed and created first.


##  sec_introduction.html \
##  sec_license.html \
##  sec_discovery.html \
##  sec_description1.html \
##  sec_description2.html \
##  sec_characteristics.html \
##  sec_community.html \
##  sec_networktools.html \
##  sec_other.html \
##  sec_thesisetc.html \
##  sec_news.html \
##  sec_software.html \
##  sec_questions.html \

EXTRA_DIST = mclmcl.txt mcx.zmm\
 stamp.day stamp.month stamp.stamp stamp.year\
 clmformat.azm clmvol.azm clmdist.azm clmmate.azm clminfo2.azm clminfo.azm clmmeet.azm\
 clm.azm clmorder.azm mcl.azm mclcm.azm\
 mclfamily.azm mclfaq.azm mcxi.azm mcx.azm mcxquery.azm mclpipeline.azm\
 mcxdump.azm mcxconvert.azm clmprotocols2.azm clmprotocols.azm mcxio.azm clmresidue.azm clmclose.azm clmoptics.azm clmimac.azm mcxassemble.azm\
 mcxalter.azm mcxerdos.azm mcxdiameter.azm mcxctty.azm mcxclcf.azm mcxrand.azm\
 mcxload.azm mcxmap.azm mcxarray.azm mcxsubs.azm index.in.azm index.html distindex.html\
 tingea.log.azm\
 $(ALIENFILES)\
 $(MAINTAINERCLEANFILES)

SUFFIXES = .1 .5 .7 .azm .html .ps .txt

.azm.1:
	$(ZOEM) -d roff -i $< -o $@
	$(ZOEM) -d roff -i $< -o $@

.azm.5:
	$(ZOEM) -d roff -i $< -o $@
	$(ZOEM) -d roff -i $< -o $@

.azm.7:
	$(ZOEM) -d roff -i $< -o $@
	$(ZOEM) -d roff -i $< -o $@

.1.ps:
	$(TROFF) -man $< > $@

.5.ps:
	$(TROFF) -man $< > $@

.7.ps:
	$(TROFF) -man $< > $@

.1.txt:
	$(TROFF) -t -e -mandoc -Tascii $< | $(COL) -bx > $@

.5.txt:
	$(TROFF) -t -e -mandoc -Tascii $< | $(COL) -bx > $@

.7.txt:
	$(TROFF) -t -e -mandoc -Tascii $< | $(COL) -bx > $@

.azm.html:
	$(ZOEM) -d html -i $< -o $@
	$(ZOEM) -d html -i $< -o $@
	- $(TIDY) -e $@

SHARE1 = "--exclude=version,h,help"
SHARE2 = "--exclude=nop,version,set,h,help,debug,test,progress,write-binary"


# clm omitted
# mcx omitted
# mcxi omitted

# ../scripts/docme --silent --exclude=version,h,help ../src/shmcl/mcl mcl.azm

.PHONY: docme-test
docme-test:
	../scripts/docme --doc-exceed=21 --silent $(SHARE1) ../src/shmcl/mcl mcl.azm
	../scripts/docme --silent $(SHARE1) ../src/shmcl/mclcm mclcm.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxdump mcxdump.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxload mcxload.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxarray mcxarray.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxrand mcxrand.azm
	../scripts/docme --silent $(SHARE1) ../src/shcl/clmformat clmformat.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxsubs mcxsubs.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxmap mcxmap.azm
	../scripts/docme --silent $(SHARE1) ../src/shmx/mcxassemble mcxassemble.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm close" clmclose.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm dist" clmdist.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm order" clmorder.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm mate" clmmate.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm meet" clmmeet.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm info" clminfo.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm info2" clminfo2.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm vol" clmvol.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm residue" clmresidue.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm imac" clmimac.azm
	../scripts/docme --silent $(SHARE2) "../src/shcl/clm optics" clmoptics.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx convert" mcxconvert.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx diameter" mcxdiameter.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx erdos" mcxerdos.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx alter" mcxalter.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx clcf" mcxclcf.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx ctty" mcxctty.azm
	../scripts/docme --silent $(SHARE2) "../src/shmx/mcx query" mcxquery.azm
	../scripts/docme --silent ../src/alien/oxygen/src/mcxdeblast ../src/alien/oxygen/doc/mcxdeblast.azm

# ../scripts/docme --silent ../src/shcl/mclpipeline mclpipeline.azm

mclindex.7: index.in.azm
	$(ZOEM) -i index.in -d roff -o mclindex.7 -s install=1
	$(TROFF) -man mclindex.7 > mclindex.ps

index.html: index.in.azm
	$(ZOEM) -i index.in -o index.html -d html -s install=1
	$(ZOEM) -i index.in -o distindex.html -d html -s dist=1


mcl: mcl.txt            mcl.1            mcl.html            mcl.ps
mclcm: mclcm.txt      mclcm.1          mclcm.html          mclcm.ps

clm:                    clm.1            clm.html            clm.ps
clminfo2:          clminfo2.1       clminfo2.html       clminfo2.ps
clminfo:            clminfo.1        clminfo.html        clminfo.ps
clmmeet:            clmmeet.1        clmmeet.html        clmmeet.ps
clmformat:        clmformat.1      clmformat.html      clmformat.ps
clmdist:            clmdist.1        clmdist.html        clmdist.ps
clmvol:              clmvol.1         clmvol.html         clmvol.ps
clmmate:            clmmate.1        clmmate.html        clmmate.ps
clmresidue:      clmresidue.1     clmresidue.html     clmresidue.ps
clmimac:            clmimac.1        clmimac.html        clmimac.ps
clmoptics:        clmoptics.1      clmoptics.html      clmoptics.ps
clmclose:          clmclose.1       clmclose.html       clmclose.ps
clmorder:          clmorder.1       clmorder.html       clmorder.ps
                                                      
mcx:                    mcx.1            mcx.html            mcx.ps
mcxi:                  mcxi.1           mcxi.html           mcxi.ps
mcxassemble:    mcxassemble.1    mcxassemble.html    mcxassemble.ps
mcxmap:              mcxmap.1         mcxmap.html         mcxmap.ps
mcxsubs:            mcxsubs.1        mcxsubs.html        mcxsubs.ps
mcxarray:          mcxarray.1       mcxarray.html       mcxarray.ps
mcxdump:            mcxdump.1        mcxdump.html        mcxdump.ps
mcxload:            mcxload.1        mcxload.html        mcxload.ps
mcxconvert:      mcxconvert.1     mcxconvert.html     mcxconvert.ps

mcxdiameter:    mcxdiameter.1    mcxdiameter.html    mcxdiameter.ps
mcxerdos:          mcxerdos.1       mcxerdos.html       mcxerdos.ps
mcxalter:          mcxalter.1       mcxalter.html       mcxalter.ps
mcxclcf:            mcxclcf.1        mcxclcf.html        mcxclcf.ps
mcxctty:            mcxctty.1        mcxctty.html        mcxctty.ps
mcxrand:            mcxrand.1        mcxrand.html        mcxrand.ps
mcxquery:           mcxquery.1      mcxquery.html       mcxquery.ps
                                                     

mclpipeline:    mclpipeline.1    mclpipeline.html    mclpipeline.ps

mcxio:                mcxio.5          mcxio.html          mcxio.ps
clmprotocols:  clmprotocols.5   clmprotocols.html   clmprotocols.ps
clmprotocols2: clmprotocols2.5 clmprotocols2.html  clmprotocols2.ps
tingea.log:		  tingea.log.7		 tingea.log.html		tingea.log.ps

mclfaq:  mclfaq.txt  mclfaq.7         mclfaq.html         mclfaq.ps	minimcl
mclfamily:        mclfamily.7      mclfamily.html      mclfamily.ps

index:             mclindex.7          index.html distindex.html


minimcl:
	cp ../scripts/minimcl .

sec_mcl.html: sec_mcl.azm
	$(ZOEM)  -i sec_mcl.azm -o sec_mcl.html -d html
	- $(TIDY) -e sec_mcl.html

sec_description1.html: sec_description1.azm
	$(ZOEM)  -i sec_description1.azm -o sec_description1.html -d html
	- $(TIDY) -e sec_description1.html

sec_description2.html: sec_description2.azm
	$(ZOEM)  -i sec_description2.azm -o sec_description2.html -d html
	- $(TIDY) -e sec_description2.html

sec_license.html: sec_license.azm
	$(ZOEM)  -i sec_license.azm -o sec_license.html -d html
	- $(TIDY) -e sec_license.html

sec_discovery.html: sec_discovery.azm
	$(ZOEM)  -i sec_discovery.azm -o sec_discovery.html -d html
	- $(TIDY) -e sec_discovery.html

sec_characteristics.html: sec_characteristics.azm
	$(ZOEM)  -i sec_characteristics.azm -o sec_characteristics.html -d html
	- $(TIDY) -e sec_characteristics.html

sec_mcledge.html: sec_mcledge.azm
	$(ZOEM)  -i sec_mcledge.azm -o sec_mcledge.html -d html
	- $(TIDY) -e sec_mcledge.html

sec_networktools.html: sec_networktools.azm
	$(ZOEM)  -i sec_networktools.azm -o sec_networktools.html -d html
	- $(TIDY) -e sec_networktools.html

sec_thesisetc.html: sec_thesisetc.azm
	$(ZOEM)  -i sec_thesisetc.azm -o sec_thesisetc.html -d html
	- $(TIDY) -e sec_thesisetc.html

sec_edge.html: sec_edge.azm
	$(ZOEM)  -i sec_edge.azm -o sec_edge.html -d html
	- $(TIDY) -e sec_edge.html

sec_other.html: sec_other.azm
	$(ZOEM)  -i sec_other.azm -o sec_other.html -d html
	- $(TIDY) -e sec_other.html

sec_questions.html: sec_questions.azm
	$(ZOEM)  -i sec_questions.azm -o sec_questions.html -d html
	- $(TIDY) -e sec_questions.html

sec_speedmemory.html: sec_speedmemory.azm
	$(ZOEM)  -i sec_speedmemory.azm -o sec_speedmemory.html -d html
	- $(TIDY) -e sec_speedmemory.html

sec_software.html: sec_software.azm
	$(ZOEM)  -i sec_software.azm -o sec_software.html -d html
	- $(TIDY) -e sec_software.html

sec_news.html: sec_news.azm
	$(ZOEM)  -i sec_news.azm -o sec_news.html -d html
	- $(TIDY) -e sec_news.html

sec_community.html: sec_community.azm
	$(ZOEM)  -i sec_community.azm -o sec_community.html -d html
	- $(TIDY) -e sec_community.html

intro.html: intro.azm
	$(ZOEM)  -i intro.azm -d html
	- $(TIDY) -e intro.html

description2.html: description2.azm
	$(ZOEM)  -i description2.azm -d html
	- $(TIDY) -e description2.html

intro3.html: intro3.azm
	$(ZOEM)  -i intro3.azm -d html
	- $(TIDY) -e intro3.html

litindex.html: litindex.azm 3Party
	$(ZOEM)  -i litindex.azm -o litindex.html -d html
	- $(TIDY) -e litindex.html

