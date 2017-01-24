.PHONY: gitstatus.txt help clean
BITS=
#BITS=-m32
#BITS=-m64
BOOST_CFLAGS=-DUSE_BOOST=1 -I./boost_1_41_0

all: help

help:
	@echo "         Usage:"
	@echo "         make" clean moses - Remake ./moses
	@echo "         make" tags - make the tags file, useful for vim 
	@false

multi:    clean tags gitstatus.txt main synthetic count_cliques

clean:
	-rm tags main count_cliques synthetic syntheticNCP *.o gitstatus.txt

tags:
	ctags *.[ch]pp

					#-Wclobbered   -Wempty-body   \ -Wignored-qualifiers  -Woverride-init   \ -Wtype-limits   -Wunused-but-set-parameter 
# I'm including most of the -Wextra flags, but I want rid of the enum-in-conditional warning from boost
CFLAGS=       \
          -Wmissing-field-initializers   \
          -Wsign-compare   \
          -Wuninitialized   \
          -Wunused-parameter    \
          -Wunused             \
          -Wall -Wformat -Werror ${BOOST_CFLAGS}

#CXXFLAGS= ${BITS}     -g
LDFLAGS+= -lgsl -lstdc++ -lgslcblas -lrt
CXXFLAGS= ${BITS} -O3     -g               ${CFLAGS} # -DNDEBUG
#CXXFLAGS=              -O2                 

boost_1_41_0:
	@echo  ""
	@echo  ""
	@echo  "    Boost 1_41_0 is required to compile moses. Download it from http://sourceforge.net/projects/boost/files/boost/1.41.0/ "
	@echo  "    and extract it in this directory. It should create a folder called boost_1_41_0."
	@echo  ""
	@echo  ""
	@false

#echo "    Extracting boost_1_41_0.tar.gz in the local directory for use by this project"
#test -d boost_1_41_0 || tar zxf boost_1_41_0.tar.gz

moses: boost_1_41_0 gitstatus.txt moses.o              overlapping.o graph_loading.o grouping.o graph_representation.o 
	g++ $(CXXFLAGS) -o moses -L. moses.o overlapping.o graph_loading.o grouping.o graph_representation.o

gitstatus.o: comment.txt  gitstatus.txt
gitstatus.txt: 
	{ git log | head -n 1 ; git status ; } | head -n 10 | sed -re 's/"/\\"/g ; s/^/"/g; s/$$/\\n"/g; ' > gitstatus.txt
