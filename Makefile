CC = g++
#CFLAGS = -O3 -Wall -Iglpk-4.2/include -D LPSOLVER_GLPK
#LIBPATH = glpk-4.2/src

CFLAGS = -O3 -Wall
LIBPATH = 
#LIBPATH = .

all: RecBlock 

BioSequenceMatrix.o: BioSequenceMatrix.cpp BioSequenceMatrix.h Utils.h
	$(CC)  $(CFLAGS) -c BioSequenceMatrix.cpp

BinaryMatrix.o: BinaryMatrix.cpp BinaryMatrix.h Utils.h
	$(CC)  $(CFLAGS) -c BinaryMatrix.cpp

GenotypeMatrix.o: GenotypeMatrix.cpp GenotypeMatrix.h Utils.h
	$(CC)  $(CFLAGS) -c GenotypeMatrix.cpp

UnWeightedGraph.o: UnWeightedGraph.cpp UnWeightedGraph.h Utils.h
	$(CC)  $(CFLAGS) -c UnWeightedGraph.cpp

Utils.o : Utils.cpp  Utils.h
	$(CC)  $(CFLAGS) -c Utils.cpp

Utils2.o : Utils2.cpp  Utils2.h Utils.h
	$(CC)  $(CFLAGS) -c Utils2.cpp

Utils3.o : Utils3.cpp  Utils3.h Utils.h
	$(CC)  $(CFLAGS) -c Utils3.cpp


SNPUtils.o : SNPUtils.cpp  SNPUtils.h Utils.h
	$(CC)  $(CFLAGS) -c SNPUtils.cpp

BlockUtils.o : BlockUtils.cpp BlockUtils.h  Utils.h
	$(CC)  $(CFLAGS) -c BlockUtils.cpp

HeuristicBlock.o : HeuristicBlock.cpp BlockUtils.h  Utils.h
	$(CC)  $(CFLAGS) -c HeuristicBlock.cpp

BlockMCMC.o : BlockMCMC.cpp BlockMCMC.h  Utils.h
	$(CC)  $(CFLAGS) -c BlockMCMC.cpp

ExactBlock.o : ExactBlock.cpp ExactBlock.h  Utils.h
	$(CC)  $(CFLAGS) -c ExactBlock.cpp

main.o: main.cpp BinaryMatrix.h Utils.h 
	$(CC)  $(CFLAGS) -c main.cpp

RecBlock: main.o BioSequenceMatrix.o BinaryMatrix.o UnWeightedGraph.o Utils.o Utils2.o \
	SNPUtils.o BlockUtils.o HeuristicBlock.o Utils3.o BlockMCMC.o ExactBlock.o GenotypeMatrix.o
	$(CC) $(CFLAGS) -o RecBlock main.o BioSequenceMatrix.o BinaryMatrix.o UnWeightedGraph.o SNPUtils.o \
	Utils.o Utils2.o BlockUtils.o  HeuristicBlock.o Utils3.o BlockMCMC.o \
	ExactBlock.o GenotypeMatrix.o

