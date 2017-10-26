#GCC Compilers:
CC  = gcc
CPP = g++
#CFLAGS   = -g -O3 -fopenmp -std=c99 
CFLAGS   = -Ofast -fopenmp -std=c99 
#CPPFLAGS = -g -O3 -fopenmp 
CPPFLAGS = -Ofast -fopenmp -std=c++0x

#Intel Compilers:
#CC  = icc
#CPP = icpc
#CFLAGS = -fast -O2 -axT -openmp # -std=c99                        #Intel Opt
#CPPFLAGS = -fast -O2 -axT -openmp # -std=c99                      #Intel Opt

#METIS_HOME = $(HOME)/metis-5.0.2
METIS_HOME = /afs/msrc.pnl.gov/files/home/hala533/metis-5.0.2
METIS_INCLUDE = -I$(METIS_HOME)/include
METIS_LIB = -L$(METIS_HOME)/libmetis -lmetis -lm

LDFLAGS  = $(CPPFLAGS)
INCLUDES = . $(METIS_INCLUDE)
LIBS     = -lm


TARGET_1 = driverForGraphClustering
TARGET_2 = convertFileToBinary
TARGET_3 = convertFileToEdgeList
TARGET_4 = driverForColoringExperiments

TARGET_5 = driverForRmat
TARGET_6 = driverForRGG
TARGET_7 = driverForPartitioningWithMetis

TARGET_8 = convertSnapFileToBinary

#TARGET   = $(TARGET_1) $(TARGET_2) $(TARGET_3) $(TARGET_4) $(TARGET_5) $(TARGET_6)

#TARGET = $(TARGET_1) $(TARGET_2) $(TARGET_3) $(TARGET_4)

TARGET = $(TARGET_1)


OBJECTS  = RngStream.o utilityFunctions.o parseInputFiles.o \
           writeGraphDimacsFormat.o buildNextPhase.o \
           coloringDistanceOne.o utilityClusteringFunctions.o \
           parallelLouvainMethod.o parallelLouvainWithColoring.o \
	   louvainMultiPhaseRun.o parseInputParameters.o vertexFollowing.o

all: $(TARGET) message

$(TARGET_1): $(OBJECTS) $(TARGET_1).o
	$(CPP) $(LDFLAGS) -o $(TARGET_1) $(TARGET_1).o $(OBJECTS) $(LIBS)

$(TARGET_2): $(OBJECTS) $(TARGET_2).o
	$(CPP) $(LDFLAGS) -o $(TARGET_2) $(TARGET_2).o $(OBJECTS) $(LIBS)

$(TARGET_3): $(OBJECTS) $(TARGET_3).o
	$(CPP) $(LDFLAGS) -o $(TARGET_3) $(TARGET_3).o $(OBJECTS) $(LIBS)

$(TARGET_4): $(OBJECTS) $(TARGET_4).o
	$(CPP) $(LDFLAGS) -o $(TARGET_4) $(TARGET_4).o $(OBJECTS) $(LIBS)

$(TARGET_5): $(OBJECTS) $(TARGET_5).o
	$(CPP) $(LDFLAGS) -o $(TARGET_5) $(TARGET_5).o $(OBJECTS) $(LIBS)

$(TARGET_6): $(OBJECTS) $(TARGET_6).o
	$(CPP) $(LDFLAGS) -o $(TARGET_6) $(TARGET_6).o $(OBJECTS) $(LIBS)

$(TARGET_7): $(OBJECTS) $(TARGET_7).o
	$(CPP) $(LDFLAGS) -o $(TARGET_7) $(TARGET_7).o $(OBJECTS) $(METIS_LIB)

$(TARGET_8): $(OBJECTS) $(TARGET_8).o
	$(CPP) $(LDFLAGS) -o $(TARGET_8) $(TARGET_8).o $(OBJECTS) $(METIS_LIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -I$(INCLUDES) -o $@

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -I$(INCLUDES) -o $@

clean:
	rm -f $(TARGET).o $(OBJECTS)

wipe:
	rm -f $(TARGET).o $(OBJECTS) $(TARGET) *~ *.bak

message:
	echo "Executables: " $(TARGET) " have been created"
