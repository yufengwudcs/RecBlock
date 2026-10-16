#ifndef UTILS_H
#define UTILS_H

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
//#include <limits>
using namespace std;


// ***************************************************************************
// Common utilities
// ***************************************************************************
//#define DEBUG(x)    cout << x
#define DEBUG(x)    

// Important structure 
typedef pair<int,int> INTERVAL;
//typedef numeric_limits<int> HAP_MAX_INT;
#define HAP_MAX_INT             0xFFFFFFF

void JoinSets( const set<int> &s1, const set<int> &s2, set<int> &res);
void SubtractSets(set<int> &s1, const set<int> &s2); 
void UnionSets(set<int> &sTotal, const set<int> &sToBeAdd);
void RmIntValFromSet( set<int> &s, int v);
void DumpIntSet(const set<int> &incSet);
void DumpIntVec(const vector<int> &intVec); 
void PopulateSetByVec( set<int> &dest, const vector<int> &srcVec); 
void PopulateVecBySet( vector<int> &dest, const set<int> &srcSet);
void CopyIntSet(set<int> & dest, const set<int> &src); 
void CopyIntVec(vector<int> & dest, const vector<int> &src); 
void CopySetIntVec( set< vector<int> > &dest, const set< vector<int> > &src); 
bool IsVecSame( const vector<int> &v1, const vector<int> &v2); 
bool IsIntVecInSet ( const set< vector<int> > &s, const vector<int> &v); 
void ConvIntToVec( unsigned int val, vector<int> &vec, int numBits);
unsigned int ConvVecToInt( const vector<int> &vec); 

void ConvIntToVecMSB( unsigned int val, vector<int> &vec, int numBits);
unsigned int ConvVecToIntMSB( const vector<int> &vec); 
void ReverseIntVec( vector<int> &vec);

unsigned int CalcBitInt( int pos, int width ); 
bool GetNextEnumVec( vector<int>& curPos, const vector<int> &limitvec); 
void YW_ASSERT(bool f);
void YW_ASSERT_INFO( bool f, const char *);
void RemoveFromIntSet( vector<int> &targetSet, int val );
bool IsIntSetEquiv(const set< vector<int> > &s1, const set< vector<int> >&s2); 
void OrderInt( int&i1, int &i2);
void SortIntVec( vector<int> &vec, int start = 0, int end = - 1);
double GetRandFraction( );
int CalcCompositeBound( map<INTERVAL, int>& mapIntervalBds, int left, int right, vector<int> &locBreakpoints );
void  OutputBounds(char *boundsFileName, map<INTERVAL, int>& mapIntervalBds, int nSites);
int ConvertToLinear(int r1, int r2, int nRows) ;
int ConvertToLinearEq(int r1, int r2, int nRows) ;


#if 0
typedef struct
{
	vector<int> posvec;		// indicate where are the choices are
	vector<int> comboChoices;	// 
} COMBO_STRUCT;
#endif

// Some utilities when doing permutations/combinations
void GetFirstCombo(int k, int n, vector<int>& posvec);
bool GetNextCombo(int k, int n, vector<int>& posvec);
bool GetNextComboFrom(int k, int n, vector<int> &posvec, int startpos);
void GetBoolVec(int num, const vector<int> &posvec, vector<bool>& bvec);
void GetIntVec(int num, const vector<int> &posvec, vector<int>& bvec);
void InitPermutation( vector<int> &nvec, const vector<int> &reference);
bool GetNextPermutation(vector<int> &nvec, const vector<int> &reference);

//************************************************************************************************
// Utilities for recombination/mutation
//************************************************************************************************
typedef vector<int> SEQUENCE;

void MutateSeqAtSite(SEQUENCE &seq, int site);
void RecombSequencesAt(const SEQUENCE &s1, const SEQUENCE &s2, int brPt, SEQUENCE &sr);
bool IsSeqRecombinnable(const SEQUENCE & s1, const SEQUENCE & s2, const SEQUENCE & st) ;
bool IsSeqRecombinnableIV(const SEQUENCE & s1, const SEQUENCE & s2, const SEQUENCE & st, INTERVAL &iv) ;
void AddUniqueSeqToVec( const SEQUENCE &seq, vector<SEQUENCE> &vecSeqs); 
bool IsSeqInVec( const SEQUENCE &seq, const vector<SEQUENCE> &vecSeqs);
bool IsSeqInSet( const SEQUENCE &seq, const set<SEQUENCE> &vecSeqs);
void GetEqualSubseq(const SEQUENCE &seq1, const SEQUENCE &seq2, int seedPos, int &left, int &right);
int  CompareSegments(const SEQUENCE &  seq, const SEQUENCE &targetSeq, int left, int right );
int IsSeqsMutPair( const SEQUENCE &seq1, const SEQUENCE &seq2);
int CalcSequencesDistance( const SEQUENCE &seq1, const SEQUENCE &seq2);
void GetNewSequences(const set<SEQUENCE>& setNewNodes, const set<SEQUENCE>& setExistingSeqs, 
					 vector<SEQUENCE>& seqNews);

//************************************************************************************************
// Utilities for haplotyping
//************************************************************************************************
void GenHapRowsSetFromGenoRows(set<int> &hapRowsSet, int numGenoRows );
bool IsTwoLabelSetsCompatible( const set<int> &partition,   const vector<int> &genoSite, bool &fZeroOne );
void GenGenoPartitions(const vector<int> &genoSite, vector<int> &part1, vector<int> &part2  );
bool Is2TwoLabelMatch(int lbla, int lblb);
bool IsTwoLabelSetContained( int genoLength, const vector<int> &setContainer,   const vector<int> &setContained );
void CalcGenoNum(int genoLength, const vector<int> &partition, vector<int> &genoNums);
int Find2LabelOccNum(int lbl, const set<int> &setUniqeLables);


#endif //UTILS_H
