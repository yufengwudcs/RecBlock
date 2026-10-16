#ifndef BLOCK_UTILS_H
#define BLOCK_UTILS_H

#include "Utils.h"
#include "Utils2.h"
#include "BinaryMatrix.h"
#include "GenotypeMatrix.h"
#include "UnWeightedGraph.h"
//#include "ILPSolver.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// important data structure
// we have a recurisve construction method
typedef vector< vector<int> > FOUNDER_LIST;


// Interface functions
int ComputeBlockLB( const BinaryMatrix &matInput, int nFounders, map<INTERVAL, int> *pBdIntervals = NULL);
int ComputeBlockGenoLB( const GenotypeMatrix &matInput, int nFounders, map<INTERVAL, int> *pBdIntervals = NULL);
int ComputeBlockHistoryLB( const BinaryMatrix &matInput, int nFounders );
int CheckSeqForFounderFrom( const FOUNDER_LIST &founder, const SEQUENCE &seq, int rowStart,
                                  set<int> &endRows);
int DerSeqFromFounderMut( FOUNDER_LIST &founder, const SEQUENCE &seq );
int DerMatrixFromFounder(  const FOUNDER_LIST &founders, const BinaryMatrix &mat);
void GetFounderState(FOUNDER_LIST &founder, const SEQUENCE &seq, vector<int> &founderBits);
double CalcNumDerFromFounders( const FOUNDER_LIST &founders, const SEQUENCE &seq, int numEvents );
int DumpParseSeqByFounders( const FOUNDER_LIST &founder, const SEQUENCE &seq );
void DumpParseMatrixByFounders( const FOUNDER_LIST &founder, const BinaryMatrix &inputMat );

// This is for test code only
void ExamWndUniqueness( int wndSize, int nFounders );
int CalcTwoFounderMinBP(const BinaryMatrix &matInput);
int TestTwoFounderMVMethod( const BinaryMatrix &matMiss );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////




#endif  // ASSOCIATION_TEST_H

