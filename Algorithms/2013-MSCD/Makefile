# Compiler (use clang++ for c++11 under OSX)
CC = g++
#CC = clang++

# Compiler flags
CFLAGS = -O3

# Uncomment below to enable C++11 (needed when USE_MULTITHREADING is enabled in config.h)
#CFLAGS += -std=c++11 -stdlib=libc++

# Linker flags
LDFLAGS =

# Include directories
INCLUDES  =-I./src -I./src/mscd/alg -I./src/mscd/ds -I./src/mscd/io/in -I./src/mscd/io/out -I./src/toolkit

# Uncomment below to add path to Armadillo (enable USE_ARMADILLO in config.h)
#INCLUDES += -I../armadillo-2.4.4/include

# Uncomment below to add support from LAPACK and BLAS (enable USE_ARMADILLO in config.h and setup Armadillo configuration file accordingly)
# LIBS = -llapack -lblas

# Source files directory
SRCPATH=src

# Sources and Objects
GEN_SRCS = $(shell find $(SRCPATH)/mscd -name '*.cpp') $(shell find $(SRCPATH)/toolkit -name '*.cpp')
GEN_OBJS = $(GEN_SRCS:.cpp=.o)
MSCD_OBJS = $(SRCPATH)/mscd_main.o $(GEN_OBJS)
ANALYSE_OBJS = $(SRCPATH)/analyse_main.o $(GEN_OBJS)
CONV_OBJS = $(SRCPATH)/conv_main.o $(GEN_OBJS)
SPEEDTEST_OBJS = $(SRCPATH)/speedtest_main.o $(GEN_OBJS)
ALL_SRCS = $(shell ls $(SRCPATH)/*.cpp) $(GEN_SRCS)
ALL_OBJS = $(ALL_SRCS:.cpp=.o)

# Targets
MSCD      = mscd
ANALYSE   = analyse
CONV      = conv
SPEEDTEST = speedtest

# Building rules

.PHONY: clean

all:	$(MSCD) $(ANALYSE) $(CONV) $(SPEEDTEST)
	@echo  Executables compiled with success

$(MSCD): $(MSCD_OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MSCD) $(MSCD_OBJS) $(LFLAGS) $(LIBS)

$(ANALYSE): $(ANALYSE_OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(ANALYSE) $(ANALYSE_OBJS) $(LFLAGS) $(LIBS)

$(CONV): $(CONV_OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(CONV) $(CONV_OBJS) $(LFLAGS) $(LIBS)

$(SPEEDTEST): $(SPEEDTEST_OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(SPEEDTEST) $(SPEEDTEST_OBJS) $(LFLAGS) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) $(ALL_OBJS) *~ $(MAIN)
