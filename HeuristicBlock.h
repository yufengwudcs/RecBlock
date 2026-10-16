#ifndef HEURISTIC_BLOCK_H
#define HEURISTIC_BLOCK_H

#include "Utils.h"
#include "Utils2.h"
#include "Utils3.h"
#include "BinaryMatrix.h"
#include "UnWeightedGraph.h"
#include <queue>
using namespace std;

#include "BlockUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// important data structure


class GreedyRecBlockParser
{
public:
    GreedyRecBlockParser( const BinaryMatrix &mat, int numFounders );
    ~GreedyRecBlockParser();

    // Now read in from a file
    int Solve( int numTrials = 1000 );
    int SolveOneCall();
    void GetSolFounders( FOUNDER_LIST &founders );

private:
    // Local data types
    typedef pair<int, INTERVAL_SUBSTRING > WEIGHTED_INTERVAL_SUBSTRING; // (score, substring)
    typedef priority_queue< WEIGHTED_INTERVAL_SUBSTRING > WT_SUBSTRING_QUEUE; 

    // implementation functons
    void BuildSubstringQueue( WT_SUBSTRING_QUEUE &substringQueue);
    void PerturbSSQueue( WT_SUBSTRING_QUEUE &curSSQueue );
    int FindHeuristicFounders (WT_SUBSTRING_QUEUE &substringQueue, FOUNDER_LIST &founders  );
    void DumpQueueEntry( const WEIGHTED_INTERVAL_SUBSTRING & entry );
    void ProcessFragment( const INTERVAL_SUBSTRING &fragment);
    bool IsFragmentResolved( const INTERVAL_SUBSTRING &fragment ) ;
    bool IsFragmentResolvedIn( const INTERVAL_SUBSTRING &fragment, int fIndex )  ;
    bool CheckFragmentForFounder( int fIndex, const INTERVAL_SUBSTRING &fragment, int &totAddedBits );
    bool AreFragmentFounderConsist( const INTERVAL_SUBSTRING &fragment, int fIndex );
    void AddFragmentToFounder( const INTERVAL_SUBSTRING &fragment, int fIndex  );
    void GetDangerSpots( set< pair<int, int> > & dangerSpots  );
    int GetFounderBit( int fIndex, int pos  );
    bool AreFounderFullyConstructed();
    void DumpFounders();
    void GetFounder( FOUNDER_LIST &founders );
    void VerifyPartialFounders();
    void FinishFounders();

    // private data
    const BinaryMatrix &inputMat;
    int numFounders;
    vector< vector<INTERVAL_SUBSTRING> > founderSubstrings;
    int numOfSteps;
    FOUNDER_LIST foundersBest;
    double maxDeviatePercent;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Useful functions

bool CheckFounders( FOUNDER_LIST &founders );
int CalcMinCrossoverBruteForce( const BinaryMatrix &inputMat, int nFounders );



#endif  // HEURISTIC_BLOCK_H

