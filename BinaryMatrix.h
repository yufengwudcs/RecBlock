#ifndef BINARY_MATRIX_H
#define BINARY_MATRIX_H

#include <list>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <map> 
using namespace std;

#include "BioSequenceMatrix.h"
#include "Utils.h"
#include "UnWeightedGraph.h"


typedef  vector < set<int>   >  COLUMN_EQUIV_CLASS;

// ***************************************************************************
// Define a reusable binary matrix class
// ***************************************************************************
class BinaryMatrix : public BioSequenceMatrix
{
public:
	BinaryMatrix();
	~BinaryMatrix(); 
	BinaryMatrix(int nr, int nc); 

	// Support assignment/copy constructor
	BinaryMatrix( const BinaryMatrix &rhs); 
	BinaryMatrix &operator=(const BinaryMatrix &rhs); 

    // Important interface functions we need
    virtual bool IsDataValid(int val) ;      // check to see if this data is good for this class
                                                // e.g. for genotype data, 0, 1, 2


	// Matrix editing functions specific to Binary (i.e. haplotype) Matrix
	void TrimDupSites(set<int>* pRemovedSites = NULL);
	int  FindDupRow(  );
	bool TrimNonInformativeSites(set<int> *pRemovedSet = NULL); 
	void TrimFullyCompatibleSites(set<int> *pRemovedSet = NULL);
	virtual void TrimNgbrDupCompSites(set<int> *pRemovedSet = NULL);

	// Matrix property checking
    bool IsColNonInformative(int c, int *singletonState);
	bool IsColTrivial (int c);
    void GetTrivialSites( vector<int> &trivSites );
	bool IsCompatible( int c1, int c2);
	virtual bool IsColComplement( int c1, int c2);
	virtual bool IsColDuplicate( int c1, int c2);
	bool IsPerfectPhylogeny(); 
	bool IsZeroColumn( int c);
	bool IsAllColumnsUnique(); 
	int GetZeroColNum();
    void GetAllIncompatiblePairs(set< pair<int,int> > &incompatibles);
    virtual int GetMajorityState( int site) ;
    int GetMinorStateNum( int site, int &minorState) const;

	// Construct interval-speceific equivalance row classes, 
	// i.e. sets of row indexes that are same
	void BuildColEquivClasses(); 
	void GetUniqueColsInRange( int c1, int c2, set<int> &setUniques  ); 
	bool IsSequencesMatch( int r1, int r2, vector<int>& seqColPos  ); 

	// Ohter utilities
	void ConstructConflictGraph( UnWeightedGraph &graph ); 
	void ConflictGraphComponents( vector<BinaryMatrix>& listSubMatrix);
	
	// Missing data utilities
	bool IsColumnBinary(int c) const;
	bool IsRowBinary(int r) const;
	void TrimNonBinaryRows();
	bool IsRowRangeBinary( int r, int left, int right  ); 

    // Lower/upper recombination bound utilities
    int ComputeHKBound();
    int ComputeFastHapBound();
    int ComputeFastRecombUpperBound();
    int ComputeMinRecombWeight( int rowIndex );
private:

	// Interval-based equivlance classes
	COLUMN_EQUIV_CLASS setColEquiv; 	
};






#endif //BINARY_MATRIX_H
