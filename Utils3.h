#ifndef UTILS3_H
#define UTILS3_H

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

#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "Utils.h"
#include "Utils2.h"

// ***************************************************************************
// HashTable
// ***************************************************************************

// Abstract class for item stored in my hash table
class YWHashItem
{
public:
    virtual ~YWHashItem() = 0;
    virtual int Key() = 0;
    virtual bool operator== (const YWHashItem &rhs) = 0;
};

// This is the hash table that mgiht be useful in some applications
// Note that this is a rather static HASH table: you can only add stuff in
// but can not remove. TBD
class YWHashTable
{
public:
    YWHashTable( int numBuckets = 100);
    ~YWHashTable(); // NOTE: has to free memory here
    void AddItem( YWHashItem *pItem );
    YWHashItem* GetIdenticalItem( YWHashItem *pItem );
    YWHashItem *GetFirstItem();
    YWHashItem *GetNextItem();
    int GetTotalItemNum() const;
    void Dump() const;

private:
    int numBuckets;
//    vector< vector<YWHashItem *> > hashTable;

    // Sorry we have not implemented hashing yet
    vector< YWHashItem * > hashTable;

    // TBD. These are for enumeation only. BUT only support single enumeration
    // PLEASE do not use in a double loop
    int curPos;
};

// ***************************************************************************
// common utilities
// ***************************************************************************
bool IsIntervalContained( const INTERVAL &iv1, const INTERVAL &iv2); 
int GetIntervalLen( const INTERVAL &iv);
int GetRandItemInSet( const set<int> &items );
int GetRandItemInVec( const vector<int> &items );
int GetWeightedRandItemInVec( const vector<int> &items, const vector<double> &itemWeights );
int GetWeightedRandItemIndex( const vector<double> &itemWeights );
// this function converts the subset over a vector to the subet over the original space
void GetOrigSubset( const vector<int> &origVec, const set<int> &subsetInd, set<int> &subsetOrig );
void MutateSequenceAtSites( SEQUENCE & mutSeq, vector<int> & mutSites );
void DumpDoubleVec(const vector<double> &vecDoubles);
bool GetFirstMutliChoice( int numStage, int numStageElem, vector<int> &initChoice );
bool GetNextMutliChoice( int numStage, int numStageElem, vector<int> &initChoice );
void DumpVecSequences( const vector<SEQUENCE> &setSeqs );
void GetVecSequencesIV( const vector<SEQUENCE> &vecSeqs, int left, int right, vector<SEQUENCE> &vecSeqsIV );
int GetNumZerosInSeq(const SEQUENCE &seq);
void GetSeqSplit(const SEQUENCE &seq, set<int> &zeroBits, set<int> &oneBits);
void SortVecIntPairs(vector<pair<int,int> > &listOfPriority );

// ***************************************************************************
// Substring utilities
// ***************************************************************************
// This file contains some extra utilties that are frequently used
typedef pair<INTERVAL, SEQUENCE> INTERVAL_SUBSTRING;    // (start,end, sequence)

int GetSubstringLeftPos( const INTERVAL_SUBSTRING &substr );
int GetSubstringRightPos( const INTERVAL_SUBSTRING &substr );
void GetIVSubstringData(const INTERVAL_SUBSTRING &substr, SEQUENCE &seq );
INTERVAL GetSubstringInterval( const INTERVAL_SUBSTRING &substr);
bool GetSubstringSegment(const INTERVAL_SUBSTRING &substr, const INTERVAL &ivToRead, SEQUENCE &segment);
int GetSubstringValAt( const INTERVAL_SUBSTRING &substr, int pos );
bool IsSegmentContained( const INTERVAL_SUBSTRING &seqContained, const INTERVAL_SUBSTRING& seqContainer );
bool AreSegmentsConsistent( const INTERVAL_SUBSTRING &seqContained, const INTERVAL_SUBSTRING& seqContainer );
int GetSegmentsIntersection( const INTERVAL_SUBSTRING &seq1, const INTERVAL_SUBSTRING& seq2, INTERVAL &iv );
bool AreSegmentsNextto( const INTERVAL_SUBSTRING &seq1, const INTERVAL_SUBSTRING& seq2 );
void DumpSubstring( const INTERVAL_SUBSTRING &substr );

// ***************************************************************************
// Other utilities
// ***************************************************************************
int FindMatchedSeqForFounders( const vector<SEQUENCE> &founder, const SEQUENCE &seq,
                                  set<int> &endRows, bool fPrefix);


#endif //UTILS3_H
