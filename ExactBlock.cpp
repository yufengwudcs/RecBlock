#include <set>
#include <vector>
using namespace std;
#include "ExactBlock.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commonly used function/variables here...

static int ENUM_BLOCK_LENGTH = 3;
static int nGreedyBkptMaxDiffAllowed = 1;
static int nMaxConfigNumStepToKeep = HAP_MAX_INT;

void SetApproxLimitControl(int nGreedyBkptLimit)
{
    nGreedyBkptMaxDiffAllowed = nGreedyBkptLimit;
}
void SetMaxConfigNumStep( int nMaxConfig)
{
    nMaxConfigNumStepToKeep = nMaxConfig;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

ExactBlockSolver :: ExactBlockSolver( const BinaryMatrix &mat, int nF, int upperBound )
:   inputMat( mat ), numFounders( nF ) , knownUpperBound(upperBound)
{
    YW_ASSERT_INFO( nF <= 10, "Sorry, this program does not support more than 10 founders" );
    Preprocess();
}

ExactBlockSolver :: ~ExactBlockSolver()
{
}


// Now read in from a file
int ExactBlockSolver :: Solve()
{
    // TBD
    return -1;
}

void ExactBlockSolver :: GetSolFounders( FOUNDER_LIST &founders )
{
    // TBD
}


// Implementations
void ExactBlockSolver :: Preprocess()
{
    // First compute lower bounds
    CalcIVLowerBound();

    // Here we start a enumeration
    // Note that any founder bit can not be all-0 or all-1
    vector<int> listFounderBits;
    for( int fb = 1; fb < (0x1 << numFounders)-1; ++fb )
    {
        listFounderBits.push_back(fb);
    }

    // Try all intervals
    int leftPos = 0;
    while( leftPos < inputMat.GetColNum() )
    {
        int rightPos = leftPos +ENUM_BLOCK_LENGTH-1;
        if( rightPos >= inputMat.GetColNum() )
        {
            rightPos = inputMat.GetColNum()-1;
        }
cout << "LeftPos = " << leftPos << ", rightpos = " << rightPos << endl;

        INTERVAL ivToCheck(leftPos, rightPos);
        // Add an entry to the candidate list of founders
        vector< vector<FOUNDER_ENCODED_VAL> > ivPossFounders;

        // Here are the submatrix
        BinaryMatrix submat;
        inputMat.SubMatrix(0, inputMat.GetRowNum()-1, leftPos, rightPos, submat);
cout << "submat = ";
submat.Dump();

        // Now we simply enumerate all possible founders for this window
        vector<int> founderBitChoice;
        int numChoiceElems = listFounderBits.size();
        int ivLength = rightPos - leftPos + 1;
        GetFirstMutliChoice( ivLength, numChoiceElems,  founderBitChoice);
        int minCross = HAP_MAX_INT;
        while(true)
        {
            FOUNDER_LIST foundersIV;
            GetFounderFromBits( foundersIV,  founderBitChoice, listFounderBits );

            int ncross = DerMatrixFromFounder( foundersIV, submat );
            if( ncross < minCross )
            {
                minCross = ncross;
            }

            // Is this founder choice good?
            if( IsFounderSegmentsOptimal( ivToCheck, foundersIV )  == true )
            {
                // Need to store this
cout << "ncross = " << ncross << endl;
cout << "Good, this founder makes sense.\n";

                // Store it
                ivPossFounders.push_back( founderBitChoice );
            }

            // Get the next choice
            if( GetNextMutliChoice( ivLength, numChoiceElems,  founderBitChoice) == false )
            {
                break;
            }
        }
        cout << "For this interval, min-crossover = " << minCross << endl;
        cout << "For this interval, number of possible founder candidates = " << ivPossFounders.size() << endl;
        YW_ASSERT_INFO( ivPossFounders.size() > 0, "At least one candidate is needed here." );
        // Now add the established list of things to 
        mapIVFounderPotBits.insert( 
            map<INTERVAL, vector< vector<FOUNDER_ENCODED_VAL> > > :: value_type(ivToCheck, ivPossFounders) ) ;


        // Now move to the next position
        leftPos += ENUM_BLOCK_LENGTH;
    }
cout << "Done with preprocess....\n";
}

int ExactBlockSolver :: CalcIVLowerBound()
{
    // First compute local lower bounds for different intervals
    map<INTERVAL, int>  localBounds;
    int lb = ComputeBlockLB( inputMat, numFounders, &localBounds);
    YW_ASSERT_INFO( lb <= knownUpperBound, "LB/UB do not match"  );

    // For now, just store the lower bounds
    this->ivLocalBounds = localBounds;

    // Now we find out to what extent we raise the local crossover
    // and it will lead to a worse solution than curretly known
    // TBD
    
    return 0;
}

bool ExactBlockSolver :: IsFounderSegmentsOptimal( const INTERVAL &ivToCheck, const FOUNDER_LIST &foundersIV  )
{
    // This function tests whether a given set of FOUNDERs for an interval can lead to a better solution than
    // known upper bound.
    // The idea is to compute an exact minimum number of crossovers for this interval and put it in the 
    // local bounds. Then computes a combined LB to see if this is smaller than upper bound. If not, no need
    // to consider such instance
    
    map<INTERVAL,int> updatedLocalBds = this->ivLocalBounds;

    // Here we compute the min-crossovers for all segments of this known piece of founders
    // for strongest possible bound we can get
    for( int ivLeft = ivToCheck.first; ivLeft < ivToCheck.second; ++ivLeft )
    {
        for(int ivRight = ivLeft +1; ivRight <= ivToCheck.second; ++ivRight )
        {
            // Now retrive a fragment of founders within this IV and see if this can be better
            int subIVLeft = ivLeft - ivToCheck.first;
            int subIVRight = ivRight - ivToCheck.first;
            vector<SEQUENCE> listSubounders;
            GetVecSequencesIV(foundersIV, subIVLeft, subIVRight, listSubounders );
            // Calc the min-crossover for this
            BinaryMatrix submat;
            inputMat.SubMatrix(0, inputMat.GetRowNum()-1, ivLeft, ivRight, submat);
            int ncross = DerMatrixFromFounder( listSubounders, submat );
            // Now store it in the list
            INTERVAL iv( ivLeft, ivRight );
            YW_ASSERT_INFO( updatedLocalBds[iv] <= ncross, "Something wrong with the lower bound" );
            updatedLocalBds[iv] = ncross;
        }
    }
    // Now compute a final lower bound here
    vector<int> localBreakpoints;
    int curLB  = CalcCompositeBound( updatedLocalBds, 0, inputMat.GetColNum()-1, localBreakpoints);
    if( curLB >= this->knownUpperBound )
    {
        return false;
    }
    else
    {
        // OK, this one may lead to a good solution
        return true;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation

void ExactBlockSolver :: GetFounderFromBits( FOUNDER_LIST & foundersIV,  const vector<int> &founderBitChoice, 
                                            const vector<int> &founderBits )
{
    foundersIV.clear();
    for( int f=0; f<numFounders; ++f )
    {
        SEQUENCE emptySeq;
        foundersIV.push_back( emptySeq );
    }
    for( unsigned int i=0; i<founderBitChoice.size(); ++i )
    {
        int c = founderBitChoice[i];
        SEQUENCE bitCol;
        GetHyperCubeSeq( founderBits[c], bitCol, numFounders );
        for( int j = 0; j<numFounders; ++j )
        {
            foundersIV[j].push_back( bitCol[j]  );
        }
    }
//cout << "The built founders = " ;
//DumpVecSequences( foundersIV );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

FounderSiteConfig:: FounderSiteConfig()
{
    // initially no dup row info
    pRowMultiplicity = NULL;
}

void FounderSiteConfig ::Dump() const
{
    cout << "site = " << site << ", founderStates = ";
    DumpIntVec( founderStates );
    cout << "number of breakpoints = ";
    DumpIntVec( numBreakpointsRows );
    cout << "inRowsMinPosInFounders = ";
    for( unsigned int i=0; i<inRowsMinPosInFounders.size(); ++i )
    {
        DumpIntSet( inRowsMinPosInFounders[i]  );
    }
}


bool FounderSiteConfig :: operator==(const FounderSiteConfig &rhs) const
{
    if( this->site != rhs.site )
    {
        return false;
    }
    if( this->founderStates != rhs.founderStates )
    {
        return false;
    }
    if( this->numBreakpointsRows != rhs.numBreakpointsRows  )
    {
        return false;
    }
    if( this->inRowsMinPosInFounders != rhs.inRowsMinPosInFounders )
    {
        return false;
    }
    return true;
}

int FounderSiteConfig :: GetMinBreakpoints() const
{
    int res = 0;
    for(unsigned int i=0; i<numBreakpointsRows.size(); ++i)
    {
        int multifac = 1;
        if( pRowMultiplicity != NULL )
        {
            multifac = (*pRowMultiplicity)[i];
        }
        res += numBreakpointsRows[i] * multifac;
    }
    return res;
}

void FounderSiteConfig :: Permuate(const vector<int> &indMapping)
{
    // for now, we only change inRowsMinPos....No other info is changed. TBD
    // permuate the rows in founders like the given maps
    for(unsigned int i=0; i<this->inRowsMinPosInFounders.size(); ++i)
    {
        set<int> rowsUpdated;
        for( set<int> :: iterator it = this->inRowsMinPosInFounders[i].begin(); it != this->inRowsMinPosInFounders[i].end(); ++it )
        {
             int convRow = indMapping[ *it ];
             rowsUpdated.insert( convRow );
        }
        inRowsMinPosInFounders[i] = rowsUpdated ;      
    }

}


int FounderSiteConfig :: CalcRelativeScore(const FounderSiteConfig &rhs)  const
{
//cout << "CalcRelativeScore: this:";
//Dump();
//cout << "rhs = ";
//rhs.Dump();
//#if 0
    // Compare this one with another config (imagine you are comparing to a min-config
    // Rule: score = base-score + correction
    // base score = min-bkpts diff
    // correction: if for a row, this contians rhs.rowset, +1, If two are identical, +0. If no containment
    // +1
    // Idea is to favor this one over rhs, be optimistiic
    int res = rhs.GetMinBreakpoints() - this->GetMinBreakpoints() ;
    for( unsigned int i=0; i< inRowsMinPosInFounders.size(); ++i )
    {
//        bool fContRhs = IsSetContainer( this->inRowsMinPosInFounders[i], 
//            rhs.inRowsMinPosInFounders[i] );
        bool fRhsCont = IsSetContainer(rhs.inRowsMinPosInFounders[i], 
            this->inRowsMinPosInFounders[i] );
        if( fRhsCont == false   )
        {
            // no contaiment
            int inc = 1;
            if( pRowMultiplicity != NULL )
            {
                inc = (*pRowMultiplicity)[i];
            }
            res += inc;
        }
//        }
//        if( fContRhs == false && fRhsCont == false  )
//        {
//            // no contaiment
//            res ++;
//        }
//        else if(fContRhs == true && fRhsCont == false )
//        {
//            res ++;
//        }
    }
//cout << "score = " << res << endl;
    return res;
//#endif

#if 0
    int res = rhs.GetMinBreakpoints() - this->GetMinBreakpoints() ;
    for( unsigned int i=0; i< inRowsMinPosInFounders.size(); ++i )
    {
//        bool fContRhs = IsSetContainer( this->inRowsMinPosInFounders[i], 
//            rhs.inRowsMinPosInFounders[i] );
        bool fRhsCont = IsSetContainer(rhs.inRowsMinPosInFounders[i], 
            this->inRowsMinPosInFounders[i] );
        if( fRhsCont == false  &&  this->numBreakpointsRows[i] <= rhs.numBreakpointsRows[i] )
        {
            // no contaiment
            res ++;
        }
//        }
//        if( fContRhs == false && fRhsCont == false  )
//        {
//            // no contaiment
//            res ++;
//        }
//        else if(fContRhs == true && fRhsCont == false )
//        {
//            res ++;
//        }
    }
//cout << "score = " << res << endl;
    return res;
#endif
}

int FounderSiteConfig :: CalcRelativeScorePerm(const FounderSiteConfig &rhs) const
{

    // Compute the best possible score by permuation of founders
//cout << "CalcRelativeScorePerm: rhs = ";
//rhs.Dump();
//cout << "This is: ";
//Dump();

    int mScore = HAP_MAX_INT;

    // Now we try all possible ways of permulations
    vector<int> refElems;
    set<int> refElemsSet;
    PopulateSetWithInterval( refElemsSet, 0, ((int) founderStates.size())-1 );
    PopulateVecBySet( refElems, refElemsSet );
    vector<int> curPerm;
    InitPermutation( curPerm, refElems);
    while( true )
    {
        // Now we test whether this is a better one
        FounderSiteConfig cfTest ;
        // we first save the zero items
        for(unsigned int i=0; i<rhs.inRowsMinPosInFounders.size(); ++i)
        {
            set<int> rowsUpdated;
            for( set<int> :: iterator it = rhs.inRowsMinPosInFounders[i].begin(); 
                it != rhs.inRowsMinPosInFounders[i].end(); ++it )
            {
                 int convRow = curPerm[ *it ];
                 rowsUpdated.insert( convRow );
            }
            cfTest.inRowsMinPosInFounders.push_back( rowsUpdated );      
            // breakpoitns are the same
            cfTest.numBreakpointsRows.push_back( rhs.numBreakpointsRows[i] );
        }
//cout << "Test rhs = ";
//cfTest.Dump();

        int score = CalcRelativeScore( cfTest);
//cout << "This score = " << score << endl;
        if( mScore > score )
        {
            mScore = score;
        }


        

        // are we done?
        if( GetNextPermutation(curPerm, refElems) == false )
        {
            break;
        }
    }
//cout << "Perm score = " << mScore << endl;
    YW_ASSERT_INFO( mScore < HAP_MAX_INT, "Did not find anyting" );
    return mScore;




#if 0
    // Compute the best possible score by permuation of founders
cout << "CalcRelativeScorePerm: rhs = ";
rhs.Dump();
cout << "This is: ";
Dump();
    set<int> zeroBitsSet;
    set<int> oneBitsSet;
    GetSeqSplit( rhs.founderStates, zeroBitsSet, oneBitsSet );
    vector<int> zeroBits;
    vector<int> oneBits;
    PopulateVecBySet( zeroBits, zeroBitsSet );
    PopulateVecBySet( oneBits, oneBitsSet );

    int mScore = HAP_MAX_INT;

    // Now we try all possible ways of permulations
    vector<int> curPerm0;
    InitPermutation( curPerm0, zeroBits);
    while( true )
    {
        // Try to permuate 1-set
        vector<int> curPerm1;
        InitPermutation( curPerm1, oneBits);
        while( true )
        {
cout << "curPerm0 = ";
DumpIntVec( curPerm0 );
cout << "curPerm1 = ";
DumpIntVec( curPerm1 );
            // what does this permuations implies
            vector<int> indMapping;
            int zeroInd = 0, oneInd = 0;
            for( unsigned int i=0; i<rhs.founderStates.size(); ++i )
            {
                if( rhs.founderStates[i] == 0 )
                {
                    // makr this to the value
                    indMapping.push_back( curPerm0[zeroInd] );
                    zeroInd ++;
                }
                else
                {
                    indMapping.push_back( curPerm1[oneInd]  );
                    oneInd ++;
                }
            }


            // Now we test whether this is a better one
            FounderSiteConfig cfTest ;
            // we first save the zero items
            for(unsigned int i=0; i<rhs.inRowsMinPosInFounders.size(); ++i)
            {
                set<int> rowsUpdated;
                for( set<int> :: iterator it = rhs.inRowsMinPosInFounders[i].begin(); 
                    it != rhs.inRowsMinPosInFounders[i].end(); ++it )
                {
                     int convRow = indMapping[ *it ];
                     rowsUpdated.insert( convRow );
                }
                cfTest.inRowsMinPosInFounders.push_back( rowsUpdated );      
                // breakpoitns are the same
                cfTest.numBreakpointsRows.push_back( rhs.numBreakpointsRows[i] );
            }
//cout << "Test rhs = ";
//cfTest.Dump();

            int score = CalcRelativeScore( cfTest);
//cout << "This score = " << score << endl;
            if( mScore > score )
            {
                mScore = score;
            }


            if( GetNextPermutation(curPerm1, oneBits) == false )
            {
                break;
            }
        }

        // are we done?
        if( GetNextPermutation(curPerm0, zeroBits) == false )
        {
            break;
        }
    }
//cout << "Perm score = " << mScore << endl;
    YW_ASSERT_INFO( mScore < HAP_MAX_INT, "Did not find anyting" );
    return mScore;
#endif
}

//const int LARGE_POSITIVE_SCORE = 1000;
//const int SMALL_NEGATIVE_SCORE = -1;

int FounderSiteConfig :: CalcRelativeScorePermApprox(const FounderSiteConfig &rhs) const
{

// simply call 
// TBD
//return CalcRelativeScore( rhs );


    // approximately calc the best relative score to rhs
//cout << "CalcRelativeScorePermApprox: rhs = ";
//rhs.Dump();
//cout << "This is: ";
//Dump();


    // when there is really no point to check, skip it
//    if( this->GetMinBreakpoints() >= rhs.GetMinBreakpoints() + GetNumInputRows()  )
//    {
//cout << "Skip 1\n";
//        return SMALL_NEGATIVE_SCORE;
//    }
//    if( this->GetMinBreakpoints() <= rhs.GetMinBreakpoints()   )
//    {
//cout << "Skip 2\n";
//        return LARGE_POSITIVE_SCORE;
//    }



    // here we assume the two states are permutations of each other
    // TBD
    int nZeros = GetNumZerosInSeq(rhs.founderStates);
    int nZerosLHS = GetNumZerosInSeq(rhs.founderStates);
    YW_ASSERT_INFO( nZeros == nZerosLHS, "Not good" );


    // We treat zero/one sepearately by first transoform zero together and one together
    // So we first normalize both part
    FounderSiteConfig normalLHS, normalRHS;
    this->Normalize(normalLHS);
    rhs.Normalize(normalRHS);
//cout << "normalLHS = ";
//normalLHS.Dump();
//cout << "normalRHS = ";
//normalRHS.Dump();

    // Now we take a simple and fast approach here
    // we simply exam the singleton case here for each input row
    typedef map< int, map<int, int> >  FC_PERM_MAP;
    FC_PERM_MAP fcMap;
    for( unsigned int r=0; r<normalLHS.inRowsMinPosInFounders.size(); ++r )
    {
        if( normalLHS.inRowsMinPosInFounders[r].size() > 1 )
        {
            // do not consider larger than 1 size for now
            continue;
        }
        if( normalRHS.inRowsMinPosInFounders[r].size() > 1 )
        {
            continue;
        }

        // Now we save this entry
        int fRowLHS = *( normalLHS.inRowsMinPosInFounders[r].begin() );
        if( fcMap.find( fRowLHS ) == fcMap.end() )
        {
            // add this entry
            map<int,int> emptyMap;
            fcMap.insert(FC_PERM_MAP:: value_type( fRowLHS, emptyMap ) );
        }
        int fRowRHS = *(normalRHS.inRowsMinPosInFounders[r].begin() );;
        if( fcMap[fRowLHS].find( fRowRHS ) == fcMap[fRowLHS].end()  )
        {
            fcMap[fRowLHS].insert(map<int,int> :: value_type( fRowRHS, 0 ) );
        }
        // now increment the count
        fcMap[fRowLHS][fRowRHS] ++;
//cout << "Find a pair of singlteons: " << fRowLHS << ", " << fRowRHS << endl;
    }
//cout << "here 1..\n";
    // Now we see whether we need to do a permuation
    vector< pair<int,int> > listOfPermChoices;
    vector< pair<int,int> > listOfPriority;
    for( FC_PERM_MAP:: iterator it1 =   fcMap.begin(); it1 != fcMap.end();  ++it1 )
    {
        for( map<int,int> :: iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2  )
        {
            pair<int,int> pp( it1->first, it2->first  );
//cout << "perm choice pp = " << pp.first << ", " << pp.second << endl;
            pair<int,int> pp2( it2->second, listOfPermChoices.size() );
//cout << "Priority choice pp2 = " << pp2.first << ", " << pp2.second << endl;
            listOfPermChoices.push_back(pp);
            listOfPriority.push_back( pp2 );
        }
    }

    // If there anything we need
    if(listOfPriority.size() == 0)
    {
        // simply return the score without exchanging
        return normalLHS.CalcRelativeScore( normalRHS );
    }
//cout << "here 2..\n";

    // create a mapping between LHS and RHS
    vector<int> indMapping;
    for( unsigned int i=0; i<rhs.founderStates.size(); ++i )
    {
        // default is directly mapping
        indMapping.push_back( -1 );
    }

    // Now we find out whether we should permuate
    SortVecIntPairs( listOfPriority );

//for(unsigned int i=0; i<(int)listOfPriority.size(); ++i)
//{
//cout << "Sorted: listOfPriority.first = " << listOfPriority[i].first << ", second = " << listOfPriority[i].second << endl;
//}
//cout << "after sorting...\n";
    // Now we start to build the choice to permutation
    set<int> itemsInSet;
    PopulateSetWithInterval( itemsInSet, 0, rhs.founderStates.size()-1 );
    set<int> fixedSet1, fixedSet2;
    bool fChange = false;
    for(  int i= (int)listOfPriority.size()-1; i>=0; --i)
    {
        // is this choice already taken
        int indChoice = listOfPriority[i].second;
//cout << "indChoice = " << indChoice <<", first = " << listOfPermChoices[indChoice].first ;
//cout << ", second = " << listOfPermChoices[indChoice].second << endl;
        if( fixedSet1.find( listOfPermChoices[indChoice].first ) != fixedSet1.end()  ||  
            fixedSet2.find( listOfPermChoices[indChoice].second ) != fixedSet2.end() )
        {
            continue;
        }

        // fix this choice
        fixedSet1.insert(listOfPermChoices[indChoice].first);
        fixedSet2.insert(listOfPermChoices[indChoice].second);
        // change mapping here
        indMapping[ listOfPermChoices[indChoice].second ] = listOfPermChoices[indChoice].first;        
        // remove the known item
        RmIntValFromSet(itemsInSet, listOfPermChoices[indChoice].first);

        if( listOfPermChoices[indChoice].first != listOfPermChoices[indChoice].second )
        {
            // this is the identical pair, no change
            fChange = true;
        }


    }

    if( fChange == false)
    {
//cout << "No change\n";
        // well, no work is need more
        return normalLHS.CalcRelativeScore( normalRHS );
    }
//cout << "here 3..\n";

    // Otherwise, we need to permuate normalRHS
    // we first cleanup the mapping in case some entries are not set properly
    vector<int> notUsedItems;
    PopulateVecBySet( notUsedItems,  itemsInSet );
    int notUsedInd = 0;
    for( unsigned int i=0; i<indMapping.size(); ++i )
    {
        if( indMapping[i] < 0 )
        {
            YW_ASSERT_INFO( notUsedInd < (int)notUsedItems.size(), "Range problem" );
            indMapping[i] = notUsedItems[notUsedInd];
            notUsedInd ++;
        }
    }
    // Now we switch the RHS
    normalRHS.Permuate(indMapping);
    return normalLHS.CalcRelativeScore( normalRHS );
}


int FounderSiteConfig :: CalcRelativeScorePermGreedy(const FounderSiteConfig &rhs) const
{
    // for now, just return simple approx. version
    //return CalcRelativeScorePermApprox(rhs);
    // greedy score is = relative diff of bkpts
    return rhs.GetMinBreakpoints() - this->GetMinBreakpoints() + nGreedyBkptMaxDiffAllowed;
}


bool FounderSiteConfig :: IsContained( const FounderSiteConfig &rhs ) const
{
    for( unsigned int i=0; i< inRowsMinPosInFounders.size(); ++i )
    {
        bool fRhsCont = IsSetContainer(rhs.inRowsMinPosInFounders[i], 
            this->inRowsMinPosInFounders[i] );
        if( fRhsCont == false  )
        {
            // no contaiment
            return false;
        }

    }
    return true;
}

void FounderSiteConfig :: Normalize(FounderSiteConfig &fcNormal) const
{
//cout << "Normalize: orig = ";
//Dump();
    // Create a copy of normalized version of it
    fcNormal.site = this->site;
    int nZero = GetNumZerosInSeq(this->founderStates);
    for( int i=0; i<nZero; ++i)
    {
        fcNormal.founderStates.push_back( 0 );
    }
    for( int i = 0; i< (int)this->founderStates.size() - nZero; ++i )
    {
        fcNormal.founderStates.push_back( 1 );
    }
    vector<int> indMapping;
    int zeroInd = 0, oneInd = 0;
    for( unsigned int i=0; i<this->founderStates.size(); ++i )
    {
        if( this->founderStates[i] == 0 )
        {
            // makr this to the value
            indMapping.push_back( zeroInd );
            zeroInd ++;
        }
        else
        {
            indMapping.push_back( nZero + oneInd );
            oneInd ++;
        }
    }


    // we first save the zero items
    for(unsigned int i=0; i<this->inRowsMinPosInFounders.size(); ++i)
    {
        set<int> rowsUpdated;
        for( set<int> :: iterator it = this->inRowsMinPosInFounders[i].begin(); it != this->inRowsMinPosInFounders[i].end(); ++it )
        {
             int convRow = indMapping[ *it ];
             rowsUpdated.insert( convRow );
        }
        fcNormal.inRowsMinPosInFounders.push_back( rowsUpdated );      
        // breakpoitns are the same
        fcNormal.numBreakpointsRows.push_back( this->numBreakpointsRows[i] );
    }

//cout << "After normalizing, ";
//fcNormal.Dump();
}

bool FounderSiteConfig :: IsLeftConfig( const FounderSiteConfig &fcRight, const BinaryMatrix &inputMat) const
{
    // test whether this config is a left (parent or earlier) config to the one passed-in
    // this can be checked by the number of breakpoints match or not
    if( fcRight.site != this->site+1 )
    {
        // site must agree
        return false;
    }
    // now get the zero and one set for next position
    set<int> zeroSites, oneSites;
    for( int r=0; r<(int)fcRight.founderStates.size(); ++r )
    {
        if( fcRight.founderStates[r] == 0 )
        {
            zeroSites.insert( r );
        }
        else
        {
            oneSites.insert( r );
        }
    }

    // Now we exaine input seq row by row to make sure they match
    for( int r=0; r<inputMat.GetRowNum(); ++r )
    {
        set<int> preExitRows = this->inRowsMinPosInFounders[r];
        set<int> curMatchedRows;
        if( inputMat(r,fcRight.site) == 0 )
        {
            curMatchedRows = zeroSites;
        }
        else
        {
            curMatchedRows = oneSites;
        }

        // Find out whether there is an intersection of current founder
        set<int> commonRows;
        JoinSets( preExitRows, curMatchedRows, commonRows );
        if( commonRows.size() == 0 )
        {
            // We start a state swtich: number of breakpoint +=1
            if( fcRight.inRowsMinPosInFounders[r] !=   curMatchedRows )
            {
//cout << "1. mismatch at r= " << r << endl;;
                return false;
            }
            
            // breakpoint number inc by 1
            if( fcRight.numBreakpointsRows[r] !=  this->numBreakpointsRows[r]+1 ) 
            {
//cout << "1. bkpt mismatch at r= " << r << endl;;
                return false;
            }
        }
        else
        {
            if( fcRight.inRowsMinPosInFounders[r] !=  commonRows )
            {
//cout << "2. mismatch at r= " << r << endl;;
                return false;
            }

            // breakpoint number stay the same
            if( fcRight.numBreakpointsRows[r] !=  this->numBreakpointsRows[r] ) 
            {
//cout << "2. bkpt mismatch at r= " << r << endl;;
                return false;
            }
        }

    }
    return true;
}

bool FounderSiteConfig :: IsLeftConfig( const FounderSiteConfig &fcRight, const GenotypeMatrix &inputMat, 
                                       vector<bool> &zeroTopFlags) const
{
//cout << "This is = ";
//this->Dump();
//cout << "fcRight = ";
//fcRight.Dump();
    // test whether this config is a left (parent or earlier) config to the one passed-in
    // this can be checked by the number of breakpoints match or not
    if( fcRight.site != this->site+1 )
    {
//cout << "site does not agrre.\n";
        // site must agree
        return false;
    }
    // now get the zero and one set for next position
    set<int> zeroSites, oneSites;
    for( int r=0; r<(int)fcRight.founderStates.size(); ++r )
    {
        if( fcRight.founderStates[r] == 0 )
        {
            zeroSites.insert( r );
        }
        else
        {
            oneSites.insert( r );
        }
    }

    // Now we exaine input seq row by row to make sure they match
    for( int r=0; r<inputMat.GetRowNum(); ++r )
    {
        int phasedR1 = 2*r;
        int phasedR2 = 2*r+1;
        set<int> preExitRows1 = this->inRowsMinPosInFounders[phasedR1];
        set<int> preExitRows2 = this->inRowsMinPosInFounders[phasedR2];
        set<int> curMatchedRows1, curMatchedRows2;

        if( inputMat(r,fcRight.site) == 0 )
        {
            curMatchedRows1 = zeroSites;
            curMatchedRows2 = zeroSites;
        }
        else if( inputMat(r,fcRight.site) == 1  )
        {
            curMatchedRows1 = oneSites;
            curMatchedRows2 = oneSites;
        }
        else
        {
            // This is a 2 here
            curMatchedRows1 = zeroSites;
            curMatchedRows2 = oneSites;
        }

        bool fFlag = true;

        if(inputMat(r,fcRight.site) != 2 )
        {
            // we only check two rows
            if( IsLeftConfigAtRow(fcRight, phasedR1, preExitRows1, curMatchedRows1) == false || 
                IsLeftConfigAtRow(fcRight, phasedR2, preExitRows2, curMatchedRows2)  == false )
            {
//cout << "1. Row r = " << r << " mismatch.\n";
                return false;
            }
        }
        else
        {
            // for a 2, we need to try both positions
            bool f1 = ( IsLeftConfigAtRow(fcRight, phasedR1, preExitRows1, curMatchedRows1) == false || 
                IsLeftConfigAtRow(fcRight, phasedR2, preExitRows2, curMatchedRows2)  == false ) ;
            bool f2 = ( IsLeftConfigAtRow(fcRight, phasedR1, preExitRows1, curMatchedRows2) == false || 
                IsLeftConfigAtRow(fcRight, phasedR2, preExitRows2, curMatchedRows1)  == false );
            if( 
                f1  &&  f2
                  
             )
            {
//cout << "Row r = " << r << " mismatch.\n";
                return false;
            }

            // remember the configuration of 2s
            if( f1 == true )
            {
                fFlag = false;
            }


        }

        // remmeer the settings
        zeroTopFlags.push_back( fFlag );
    }
    return true;
}


bool FounderSiteConfig :: IsLeftConfigAtRow( const FounderSiteConfig &fcRight, int r, const set<int> &leftRowSet, 
    const set<int> &rightRowSet ) const
{
    set<int> commonRows;
    JoinSets( leftRowSet, rightRowSet, commonRows );
//cout << "r = " << r << ", where inRowsMinPosInFounders = ";
//DumpIntSet( fcRight.inRowsMinPosInFounders[r] ); 
//cout << "leftRows = ";
//DumpIntSet(leftRowSet);
//cout << "rightRows = ";
//DumpIntSet(rightRowSet);
//cout << "commonrows = ";
//DumpIntSet( commonRows );

    if( commonRows.size() == 0 )
    {
        // We start a state swtich: number of breakpoint +=1
        if( fcRight.inRowsMinPosInFounders[r] !=   rightRowSet )
        {
//cout << "1. mismatch at r= " << r << endl;;
            return false;
        }
        
        // breakpoint number inc by 1
        if( fcRight.numBreakpointsRows[r] !=  this->numBreakpointsRows[r]+1 ) 
        {
//cout << "1. bkpt mismatch at r= " << r << endl;;
            return false;
        }
    }
    else
    {
        if( fcRight.inRowsMinPosInFounders[r] !=  commonRows )
        {
//cout << "2. mismatch at r= " << r << endl;;
            return false;
        }

        // breakpoint number stay the same
        if( fcRight.numBreakpointsRows[r] !=  this->numBreakpointsRows[r] ) 
        {
//cout << "2. bkpt mismatch at r= " << r << endl;;
            return false;
        }
    }
    // yes match this row
    return true;
}


bool FounderSiteConfig :: IsIsomorphismTestOK( const vector<int> &founderState) const
{
    // if the block number is equal to the size
    if( founderState.size() ==  listUniRowBlocks.size() )
    {
        return true;
    }

    // test whether this new site fits the config
    // to pass symetry, in each block, the 1 must follow all 0 in the same block
    // that is, if a block has a 1, then all the following must all be 1
    int startPos = 0;
    for( unsigned int i=0; i<listUniRowBlocks.size(); ++i )
    {
        bool fSeenOne = false;
        // test for each block
        for(int j=0; j<listUniRowBlocks[i]; ++j)
        {
            // 
            if( founderState[startPos] == 0 )
            {
                if( fSeenOne == true )
                {
                    return false;
                }
            }
            else
            {
                fSeenOne = true;
            }

            startPos++;
        }
    }
    return true;
}

void FounderSiteConfig :: InitUnrestrictedUniBlock(const vector<int> &founderState) 
{
    // simply use stretch of 0 or 1 blocks 
    listUniRowBlocks.clear();
    int szBlk = 1;
    int curBit = founderState[0];
    for( unsigned int p=1; p<founderState.size(); ++p )
    {
        if( founderState[p] == curBit  )
        {
            szBlk ++;
        }
        else
        {
            listUniRowBlocks.push_back( szBlk );
            curBit = founderState[p];
            szBlk = 1;
        }
    }
    // save the last block
    listUniRowBlocks.push_back( szBlk );
}

void FounderSiteConfig :: InitRestrictedUniBlock( const vector<int> &founderState, 
                                             const FounderSiteConfig& prevFounderConfig) 
{
    // Idea: break down each block into two blocks if needed
    listUniRowBlocks.clear();

    unsigned int offsetPos=0;
    for( unsigned int ii=0; ii<prevFounderConfig.listUniRowBlocks.size(); ++ii )
    {
        int szBlk = 1;
        int curBit = founderState[offsetPos];

        // test for each segment to see if we need to breakup
        for( int p=1; p< prevFounderConfig.listUniRowBlocks[ii]; ++p )
        {
            // if at the end of one block, then should output it
    
            if( founderState[p+offsetPos] == curBit  )
            {
                szBlk ++;
            }
            else
            {
                listUniRowBlocks.push_back( szBlk );
                curBit = founderState[p+offsetPos];
                szBlk = 1;
            }
        }
        // save the last block
        listUniRowBlocks.push_back( szBlk );

        // adjust offsetpos
        offsetPos += prevFounderConfig.listUniRowBlocks[ii];
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

void DumpVecFConfig( const vector<FounderSiteConfig> &vecFCs )
{
    cout << "Totally number of FCs = " << vecFCs.size() << endl;
    for( unsigned int i=0; i<vecFCs.size(); ++i )
    {
        vecFCs[i].Dump();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
ExactEnumFoundersAbstractSolver :: ExactEnumFoundersAbstractSolver()
    : fDispInfo( true )
{
    // by default, approx is off
    this->fApproxFlag = false;
//    randSavedSolNum = 100;  // try to save random 100 solutions

}

int ExactEnumFoundersAbstractSolver :: GetMinBreakpoints( const vector<FounderSiteConfig> &prevSiteFounderConfigs,
                                                 vector<int> *pIndList, bool fPerm)
{
    // This is the min-crossovers we need
    int res = HAP_MAX_INT;
    if(pIndList != NULL)
    {
        pIndList->clear(); 
    }
    int indMin = -1;
    for( unsigned int i=0; i<prevSiteFounderConfigs.size(); ++i )
    {
//cout <<"GetMinBreakpoints: fc = ";
//prevSiteFounderConfigs[i].Dump();
        int stepRes = 0;
        for(unsigned int r=0; r< prevSiteFounderConfigs[i].numBreakpointsRows.size(); ++r)
        {
            stepRes += prevSiteFounderConfigs[i].numBreakpointsRows[r] * GetMultiplicity(r);
        }
        if( stepRes < res )
        {
            res = stepRes;
            indMin = i;
        }

//#if 0
        else if( stepRes == res   )
        {
            YW_ASSERT_INFO( indMin  >=0, "Something very wrong" );
            // In case of a match, we want to pick the one as general as possible
           
            if( fPerm == false &&   prevSiteFounderConfigs[indMin].IsContained(prevSiteFounderConfigs[i]) == true )
            {
                // choose this instead
                indMin = i;
            }
//            else if( fPerm == true && prevSiteFounderConfigs[i].CalcRelativeScorePerm(prevSiteFounderConfigs[indMin]) <= 0   )
            else if( fPerm == true && prevSiteFounderConfigs[i].CalcRelativeScorePermApprox(prevSiteFounderConfigs[indMin]) <= 0   )
            {
                indMin = i;
            }
        }
//#endif
    }

    // When we want to find a list of best founders, we do this
    if( pIndList != NULL )
    {
        YW_ASSERT_INFO( indMin >=0, "foo" );
        pIndList->push_back( indMin );


        // Now we find out other possible candidates
        for( unsigned int i=0; i<prevSiteFounderConfigs.size(); ++i )
        {
            if( prevSiteFounderConfigs[i].GetMinBreakpoints() > res  )
            {
                // not good
                continue;
            }

            if( pIndList != NULL  )
            {
                bool fAdd = true;
                // Make susre there are no better ones
                for( unsigned int ff = 0; ff< pIndList->size(); ++ff )
                {
                    // Is this better?
                    int nIndex = (*pIndList)[ff];
                    if(  fPerm == false && prevSiteFounderConfigs[i].IsContained(prevSiteFounderConfigs[nIndex]) == true )
                    {
                        // no need to add this
                        fAdd = false;
                        break;
                    }
                    else if( fPerm == true && 
                        prevSiteFounderConfigs[indMin].CalcRelativeScorePermApprox(prevSiteFounderConfigs[i]) <= 0  )
//                        prevSiteFounderConfigs[indMin].CalcRelativeScorePerm(prevSiteFounderConfigs[i]) <= 0  )
                    {
                        fAdd = false;
                        break;
                    }
                }
                if( fAdd == true)
                {
                    pIndList->push_back(i);
                }
            }
        }
    }


    if( res == HAP_MAX_INT)
    {
        res = -1;
    }

    return res;
}


void ExactEnumFoundersAbstractSolver :: FindFounderConfigs( int site, const vector<int> &founderNPState, 
                        const vector<FounderSiteConfig> &prevSiteFounderConfigs, 
                        vector<FounderSiteConfig> &curFoundersConfigs )
{
    // Find out which founders can be of use here
    // Try every stored config for last site
    // NOTE: founderState can be swapped into multiple real state here
//cout << "prevSiteFounderConfigs.size() = " << prevSiteFounderConfigs.size() << endl;

    vector<FounderSiteConfig> stepFConfigs;
    vector<int> refElems = founderNPState;
    vector<int> curElems;
    InitPermutation(curElems, refElems);

    while (true )
    {
//cout << "Now work with permutation: ";
//DumpIntVec( curElems );
        vector<FounderSiteConfig> subStepFConfigs;
        for( unsigned int i = 0; i< prevSiteFounderConfigs.size(); ++i)
        {
            // ensure symetry breaking here
            if( prevSiteFounderConfigs[i].IsIsomorphismTestOK( curElems )  == true )      // some bounding here
            {
                vector<FounderSiteConfig> fConfigList;

                // Now get the inferred min-state
                //vector<set<int> > rowsFounderPos;
//cout << "Pre-config = ";
//prevSiteFounderConfigs[i].Dump();
                BuildFounderFromPre( site, prevSiteFounderConfigs[i], curElems, fConfigList  );


                // now construct the partitions for each one
                for(unsigned int ppp = 0; ppp < fConfigList.size(); ++ppp)
                {
                    fConfigList[ppp].InitRestrictedUniBlock( curElems, prevSiteFounderConfigs[i] );
                }

                //curFoundersConfigs.push_back( fConfig );
//cout << "Adding entries : ";
//DumpVecFConfig( fConfigList );
                ScreenAppendConfig( subStepFConfigs, fConfigList );
            }
        }
//cout << "Raw subStepFConfigs.size() = " << subStepFConfigs.size() << endl;
        // Now prune it first
//cout << "Before step prune, subStepFConfigs.size() = " << subStepFConfigs.size() << endl;
        vector<int> dummyVec;
        PruneFounderConfigStep(subStepFConfigs, dummyVec);
//cout << "After step prune, subStepFConfigs.size() = " << subStepFConfigs.size() << endl;
        // Now add it into final list
        // Now we want to append the founders found so far
        for( unsigned int i=0; i<subStepFConfigs.size(); ++i )
        {
            stepFConfigs.push_back( subStepFConfigs[i] );
        }

        if( GetNextPermutation(curElems, refElems) == false )
        {
            break;
        }
    }
    // Prune it first
//cout << "BEFORE perm prune, stepFConfigs.size() = " << stepFConfigs.size() << endl;

//    PruneFounderConfigStepSwap(stepFConfigs);
    PruneFounderConfigStepPerm(stepFConfigs);
//cout << "AFTER perm prune, stepFConfigs.size() = " << stepFConfigs.size() << endl;
    // Now we want to append the founders found so far
    for( unsigned int i=0; i<stepFConfigs.size(); ++i )
    {
        curFoundersConfigs.push_back( stepFConfigs[i] );
    }
//cout << "Done with one.\n";
}

void ExactEnumFoundersAbstractSolver ::  ScreenAppendConfig( vector<FounderSiteConfig> &stepFConfigs, 
                                                    const vector<FounderSiteConfig>& fConfigList )
{
    if(fConfigList.size() == 0)
    {
        return;
    }

    // if this partial-built founders already use too much breakpoints, do not conitnue
    for( unsigned ff = 0; ff< fConfigList.size(); ++ff)
    {
        if( IsOptPossibleWithPartialSol(0, fConfigList[ff].site, fConfigList[ff].GetMinBreakpoints() ) == false )
        {
    //cout << "This can not be a good solution.\n";
            continue;
        }
    //#if 0
        // simply chec
        bool fAdd = true;
        for( unsigned int i=0; i<stepFConfigs.size(); ++i )
        {
            // If this one matches eactaly one of existing one, do nothing
            if( fConfigList[ff] ==  stepFConfigs[i] )
            {
//cout << "**********Duplicate. Skip it\n";
//fConfigList[ff].Dump();
                fAdd = false;
                break;
            }
        }
        // Otherwise, insert it
        if( fAdd == true )
        {
            stepFConfigs.push_back( fConfigList[ff] );
        }
    //#endif
    }

#if 0
#if 0
cout << "ScreenAppendConfig: fConfig = ";
fConfig.Dump();
cout << "Size of current list = " << stepFConfigs.size() << endl;
for( unsigned int i=0; i<stepFConfigs.size(); ++i )
{
stepFConfigs[i].Dump();
}
#endif
    // We need to figure out whether this config make any sense to add
    // Rule: if we already have a known better config, do not append it
    // Better means at most same amount of bkpts
    vector<FounderSiteConfig> screenedFConfigs;

    bool fAddFConfig = true;
    for( unsigned int i=0; i<stepFConfigs.size(); ++i )
    {
        // If this one matches eactaly one of existing one, do nothing
        if( fConfig ==  stepFConfigs[i] )
        {
//cout << "Duplicate. Skip it\n";
            return;
        }

        if(  IsFounderBetterThan( fConfig, stepFConfigs[i]  ) == false )
        {
//cout << "Save this one: \n";
//stepFConfigs[i].Dump();
            // We need to save this
            screenedFConfigs.push_back( stepFConfigs[i]  );
        }
        else
        {
//cout << "This existing one is not useful.\n";
//stepFConfigs[i].Dump();
        }
        // If this existing one if better than fConfig, do not add
        if( IsFounderBetterThan( stepFConfigs[i],  fConfig)  == true ) 
        {
            fAddFConfig = false;
        }
    }
    // We save this fConfig if there is no one clearly better than
    if( fAddFConfig == true )
    {
        // also test whether this one use too much crossovers
        //if( IsOptPossibleWithPartialSol(0, fConfig.site, fConfig.GetMinBreakpoints() ) == true )
        {
//cout << "This fConfig is a GOOD one . We keep it.\n";
            screenedFConfigs.push_back( fConfig );
        }
    }
    else
    {
//cout << "This fConfig is REDUNDENT.\n";
    }
    // Now save it back
   stepFConfigs = screenedFConfigs;
#endif
}

void ExactEnumFoundersAbstractSolver :: FindMatchedFounderRows( int bitVal, 
                                                               const vector<int> &founderState,  set<int> &fRows )
{
    // Which founder bit 
    fRows.clear();
    for( unsigned int i=0; i <founderState.size(); ++i )
    {
        if( founderState[i] == bitVal )
        {
            fRows.insert( i );
        }
    }
}

//#define PRUNE_FAST_STEP_PROC  1
#define PRUNE_FAST_STEP_PROC_THRHOLD  50

void ExactEnumFoundersAbstractSolver :: PruneFounderConfigStep(vector<FounderSiteConfig> &foundersConfigs, vector<int> &keptPos)
{
//return;
//cout << "foundersConfigs: entry num = " << foundersConfigs.size() << endl;
//DumpVecFConfig(foundersConfigs);
    keptPos.clear();
    if( foundersConfigs.size() == 0)
    {
        // If there is nothing to try, skip
        return;
    }

    // do a more time-consuming way, where the number of item si not too large
//    if( foundersConfigs.size() < PRUNE_FAST_STEP_PROC_THRHOLD )
//    {
#ifndef PRUNE_FAST_STEP_PROC
    //cout << "foundersConfigs size " << foundersConfigs.size() << endl;
        vector<FounderSiteConfig> updateFConfig;
        set<int> posKeptSet;

        // we want to sort the list by their minBkpts so far
        vector<pair<int,int> > listOfPriority;
        for(unsigned int i=0; i<foundersConfigs.size(); ++i)
        {
            // 
            pair<int,int> pp;
            pp.first = foundersConfigs[i].GetMinBreakpoints();
            pp.second = i;
            listOfPriority.push_back( pp );
        }
        SortVecIntPairs( listOfPriority );

//cout << "Here1.1.1.1.1\n";
        // we require the 
//        for( unsigned int i=0; i<foundersConfigs.size(); ++i )
        for( unsigned int i=0; i<listOfPriority.size(); ++i )
        {
            int iid = listOfPriority[i].second;
            bool fAdd = true;
            //unsigned int j=0;
            //for( j = i+1; j<foundersConfigs.size(); ++j)
            for(unsigned j=0; j<updateFConfig.size(); ++j)
            {
                int score;
                if( fApproxFlag == false )
                {
                    //score = foundersConfigs[i].CalcRelativeScorePermApprox( foundersConfigs[ j ] );
                    score = foundersConfigs[iid].CalcRelativeScorePermApprox( updateFConfig[ j ] );
                }
                else
                {
//cout << "Greedy here\n";
                    //score = foundersConfigs[i].CalcRelativeScorePermGreedy( foundersConfigs[ j ] );
                    score = foundersConfigs[iid].CalcRelativeScorePermGreedy( updateFConfig[ j ] );
                }

                if( score <= 0 )
                {
                    fAdd = false;
                    break;
                }
            }
            if( fAdd == true  )
            {
                updateFConfig.push_back( foundersConfigs[iid] );

                // keet this
                posKeptSet.insert( iid );
            }
            else
            {
//cout << "++++++++This config is not added: \n";
//foundersConfigs[i].Dump();
//cout << "due to \n";
//foundersConfigs[j].Dump();
//cout << i << " is not added here for founder " << f << ".\n";

            }
        }

        // return
        PopulateVecBySet( keptPos, posKeptSet );
        foundersConfigs = updateFConfig;

//    }
//    else
//    {


#else
        // This function prunes a list of founder configurations for a single state
        vector<int> indMinList;
        GetMinBreakpoints(foundersConfigs, &indMinList);
        YW_ASSERT_INFO(indMinList.size() >0, "Something wrong");
    //cout << "PruneFounderConfigStep: indMinList = ";
    //DumpIntVec( indMinList );

        vector<FounderSiteConfig> updateFConfig;
        set<int> posKeptSet;
        // first save the minimum ones
        for( unsigned int f=0; f<indMinList.size(); ++f )
        {
            updateFConfig.push_back(foundersConfigs[ indMinList[f] ]);

            // markt hese as kept
            posKeptSet.insert( indMinList[f]  );
        }

        // we require the 
        for( unsigned int i=0; i<foundersConfigs.size(); ++i )
        {
            bool fAdd = true;
            unsigned int f=0;
            for( f=0; f<indMinList.size(); ++f)
            {
//                int score = foundersConfigs[i].CalcRelativeScoreApprox( foundersConfigs[indMinList[f] ] );
                int score;
                if( fApproxFlag == false )
                {
                    score = foundersConfigs[i].CalcRelativeScorePermApprox( foundersConfigs[ indMinList[f]  ] );
                }
                else
                {
                    score = foundersConfigs[i].CalcRelativeScorePermGreedy( foundersConfigs[ indMinList[f]  ] );
                }
                if( score <= 0 )
                {
                    fAdd = false;
                    break;
                }
            }
            if( fAdd == true )
            {
                updateFConfig.push_back( foundersConfigs[i] );

                // keet this
                posKeptSet.insert( i );
            }
            else
            {
    //cout << i << " is not added here for founder " << f << ".\n";
            }
        }

        // return
        PopulateVecBySet( keptPos, posKeptSet );
        foundersConfigs = updateFConfig;
    //cout << "done with step prune\n";
    //exit(1);
#endif
    //}
//cout << "foundersConfigs: exit num = " << foundersConfigs.size() << endl;
//DumpVecFConfig(foundersConfigs);
}

void ExactEnumFoundersAbstractSolver :: PruneFounderConfigStepPerm(vector<FounderSiteConfig> &foundersConfigs)
{
    // In case there is nothing to purne, do nothing
    if(foundersConfigs.size() == 0)
    {
        return;
    }

#ifndef PRUNE_FAST_STEP_PROC

    // We simply test each one of them to another
    // This function prunes a list of founder configurations for a single state

    vector<FounderSiteConfig> keptFConfig;

    // we require the 
    for( unsigned int i=0; i<foundersConfigs.size(); ++i )
    {
        bool fAdd = true;
        unsigned int j;
        for(j=i+1; j<foundersConfigs.size(); ++j)
        {
//            int score = foundersConfigs[i].CalcRelativeScorePerm( foundersConfigs[indMinList[f] ] );
            int score = foundersConfigs[i].CalcRelativeScorePermApprox( foundersConfigs[ j ] );
            if( score <= 0 )
            {
                fAdd = false;
                break;
            }
        }
        if( fAdd == true )
        {
            keptFConfig.push_back( foundersConfigs[i] );

        }
        else
        {
//cout << "--------This config is not added: \n";
//foundersConfigs[i].Dump();
//cout << "due to \n";
//foundersConfigs[j].Dump();

        }
//exit(1);
    }

    // return
    foundersConfigs = keptFConfig;
YW_ASSERT_INFO(foundersConfigs.size() > 0, "Can not be all pruned");


#else
    // we simply normailize every config
    vector<FounderSiteConfig> updatedFConfigs;
    for( unsigned int i=0; i<foundersConfigs.size(); ++i )
    {
        FounderSiteConfig fcn;
        foundersConfigs[i].Normalize(fcn);
        updatedFConfigs.push_back( fcn );
    }


    // We simply test each one of them to another
    // This function prunes a list of founder configurations for a single state
    vector<int> indMinList;
    GetMinBreakpoints(updatedFConfigs, &indMinList );
    YW_ASSERT_INFO(indMinList.size() >0, "Something wrong");
//cout << "PruneFounderConfigStepPerm: indMinList = ";
//DumpIntVec( indMinList );
//exit(1);
    vector<FounderSiteConfig> keptFConfig;
    set<int> posKeptSet;
    // first save the minimum ones
    for( unsigned int f=0; f<indMinList.size(); ++f )
    {
        keptFConfig.push_back(foundersConfigs[ indMinList[f] ]);

    }
    // we require the 
    for( unsigned int i=0; i<foundersConfigs.size(); ++i )
    {
        bool fAdd = true;
        for(unsigned int f=0; f<indMinList.size(); ++f)
        {
//            int score = foundersConfigs[i].CalcRelativeScorePerm( foundersConfigs[indMinList[f] ] );
            int score = foundersConfigs[i].CalcRelativeScorePermApprox( foundersConfigs[indMinList[f] ] );
            if( score <= 0 )
            {
                fAdd = false;
                break;
            }
        }
        if( fAdd == true )
        {
            keptFConfig.push_back( foundersConfigs[i] );

        }
//exit(1);
    }

    // return
    foundersConfigs = keptFConfig;
YW_ASSERT_INFO(foundersConfigs.size() > 0, "Can not be all pruned");
#endif
}


int ExactEnumFoundersAbstractSolver :: GetMultiplicity( int r ) 
{
    // by default, return 1 to indicate no multiplicity
    return 1;  
}


///////////////////////////////////////////////////////////////////////////////////////////
// DFS functions

void ExactEnumFoundersAbstractSolver :: InitDFS(int numSites)
{
    // Now we initialize explored/unexplored items
    for( int i=0; i<numSites; ++i  )
    {
        vector<FounderSiteConfig> initStore;
        unexploredFCStore.push_back( initStore );
        vector<FounderSiteConfig> initExpStore;
        exploredFCStore.push_back(initExpStore);
    }
}

void ExactEnumFoundersAbstractSolver :: MarkExplored( const FounderSiteConfig *pFCExplored  )
{
    YW_ASSERT_INFO(false, "Degenrate call");
#if 0
    int site = fcExplored.site;
    exploredFCStore[site].push_back( fcExplored  );

    // Now remove it from the unexplored
    for( map<FounderSiteConfig, bool> :: iterator it = unexploredFCStore[site].begin(); 
        it != unexploredFCStore[site].end(); ++it )
    {
        unexploredFCStore[site].erase( fcExplored );
    }
#endif
}

const FounderSiteConfig *ExactEnumFoundersAbstractSolver:: GetNextToExplore(int site  )
{
    // CAUTION: must call this 
    // We want to find the item with the lowest number of breakpoints
    int minBkpt = HAP_MAX_INT;
//    const FounderSiteConfig *pFC = NULL;
    vector<FounderSiteConfig> :: iterator itToRm;
    for( vector<FounderSiteConfig > :: iterator it = unexploredFCStore[site].begin(); 
        it != unexploredFCStore[site].end(); ++it )
    {
        // Now we test it
        int curbkpt = it->GetMinBreakpoints();
        if(  curbkpt < minBkpt  )
        {
            minBkpt = curbkpt;
//            pFC = &(*it);
            itToRm = it;
        }
    }
    //YW_ASSERT_INFO( pFC != NULL, "Something wrong" );

    // mark it as done
 //   MarkExplored(pFC);

    // Now we remove the item from consideration
    exploredFCStore[site].push_back( *itToRm  );
    unexploredFCStore[site].erase( itToRm );

    // result is the last in the explored item
    return &exploredFCStore[site][ (int)exploredFCStore[site].size()-1 ];
}

bool ExactEnumFoundersAbstractSolver :: AreMoreFCToExplore(int site)
{
    if( unexploredFCStore[site].size() > 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}


void ExactEnumFoundersAbstractSolver :: AppendFCToExplore( int site, const vector<FounderSiteConfig> &listFCToExp)
{
    // append these candidates to the stack, waiting to be explored
    for( unsigned int i=0; i<listFCToExp.size(); ++i )
    {
        if( IsFCExcluded( listFCToExp[i]  ) == false )
        {
            unexploredFCStore[site].push_back(  listFCToExp[i]  );
        }
    }
}

bool ExactEnumFoundersAbstractSolver :: IsFCExcluded( const FounderSiteConfig &fc )
{
    int site = fc.site;
    // we need to check whether there is any explored FC 
    // that is at least as good as this one
    for(  unsigned int i=0; i<exploredFCStore[site].size(); ++i)
    {
        if( fc.CalcRelativeScorePermApprox( exploredFCStore[site][i] ) <= 0 )
        {
//cout << "IsFCExcluded: fc = ";
//fc.Dump();
//cout << "Being tested = ";
//exploredFCStore[site][i].Dump();
//exit(1);
            return true;
        }
    }
    return false;
}

// Solve this problem by a DFS manor
int ExactEnumFoundersAbstractSolver :: SolveDFS()
{
    // First initialize DFS
    InitDFS( GetDataWidth()   );

    // Herw are the possible founder bits
    vector<int> listFounderBits;
    for( int fb = 1; fb < GetNumFounders(); ++fb )
    {
        int founderBit = (0x1 << fb) - 1;
        listFounderBits.push_back(founderBit);
    }

    // Setup the candidates for the first site
    // We start by having a un-restricted founder site as start for site 0
    vector<FounderSiteConfig> site0FoundersConfig;

    // Keep track of, for each stage, the min-breakpoints we need
    // The first site = 0
//    minBkptsForSites.push_back( 0 );

    // Now we enumerate all possible founder states
    for( int i=1; i<GetNumFounders(); ++i )
    {
        // we only need to try 00001, 00011, 00111, 01111 type
        int founderBit = (0x1 << i) - 1;

        vector<int> founderState;
        GetHyperCubeSeq(founderBit, founderState, GetNumFounders() );
//cout << "Trying founder state: ";
//DumpIntVec( founderState );
        FounderSiteConfig fConfig;
        GetUnRestrictedFounderConfigAt(0, founderState, fConfig);

        site0FoundersConfig.push_back( fConfig );
    }

    // Store these as candidates
    AppendFCToExplore(0, site0FoundersConfig);
//cout << "After appending 0-site FC..\n";
    // Now start to handle the 1, 2, 3, ...., m-1 sites
    int siteToExplore = 0;
    while(true )
    {
//cout << "Processing site = " << siteToExplore << endl;

        // Check to see if there are more to explore
        if( AreMoreFCToExplore(siteToExplore) == false   )
        {
            if(  siteToExplore == 0  )
            {
                // in this case, we have nothing to continue;
                break;
            }
            else
            {
                // Now move backwards by one
                siteToExplore --;
                continue;
            }
        }
//cout << "Thre are still more to explore...\n";

        // Now get next FC to explore
        const FounderSiteConfig *pFCToExp = GetNextToExplore( siteToExplore );
//cout << "Next to explore is: ";
//pFCToExp->Dump();
        YW_ASSERT_INFO( pFCToExp != NULL, "Can not be nothing" );

//cout << "foundersConfigs.size() = " << foundersConfigs.size() << endl;
        // Are we done for this path
        // Now we only explore from the current one, that is the difference
        vector<FounderSiteConfig> singleFCItem;
        singleFCItem.push_back( *pFCToExp );

        if( siteToExplore < GetDataWidth() -1 )
        {

            vector<FounderSiteConfig> foundersConfigs;
            for( unsigned int i=0; i<listFounderBits.size(); ++i )
            {
                vector<int> founderState;
                GetHyperCubeSeq(listFounderBits[i], founderState, GetNumFounders() );

                FindFounderConfigs( siteToExplore+1, founderState, singleFCItem, foundersConfigs );
            }

            // add these FC to store
            AppendFCToExplore(siteToExplore+1, foundersConfigs);

            // Now we move downwards by 1
            siteToExplore ++;
        }
        else
        {
            // Get the min bkpts from the current configs
            int curMinBkpts = GetMinBreakpoints( singleFCItem   );
            if( curMinBkpts < GetUpperBound() )
            {
                cout << "Now we find a better solution, whose number of breakpoints = " << curMinBkpts << endl;
                SetUpperBound( curMinBkpts );
            }

            // Nnow move backwards
            siteToExplore --;
        }
    }

    // Save the founder
    BuildDFSSolFounders(  );


    // here is the best we have
    return GetUpperBound() ;

}

void ExactEnumFoundersAbstractSolver :: BuildDFSSolFounders( )
{
    //
cout << "Now build founders...\n";
    optDFSFCs.clear();
cout << "Pass here0.0.0.0\n";

    // We trace from backwards to find the minimum one on the rightmost parse
    // If there is nothing at the rightmost entry, no founders are found
    if(exploredFCStore[ GetDataWidth() -1 ].size() == 0)
    {
        cout << "Fail to find good founders2.\n";
        return;
    }
cout << "Pass here0.0\n";
    vector<int> minList;
    GetMinBreakpoints(exploredFCStore[ GetDataWidth()-1 ], &minList);
    YW_ASSERT_INFO( minList.size() > 0, "Must find at least one" );
    vector<SEQUENCE> foundersBeingBuilt;
    for(int f=0; f<GetNumFounders();++f)
    {
        SEQUENCE emptySeq;
        foundersBeingBuilt.push_back(emptySeq);
    }
cout << "Pass here\n";
    // For now, pick the first one to continue
    int fInd = minList[0];
    FounderSiteConfig lastFConfig = exploredFCStore[ GetDataWidth()  - 1 ][fInd];

    // save it
    vector<FounderSiteConfig> reversedCFs;
    reversedCFs.push_back(lastFConfig);

    for(  int i=0; i<GetNumFounders(); ++i )
    {
        // Save the last bit first
        foundersBeingBuilt[i].push_back( exploredFCStore[ GetDataWidth()-1 ][fInd].founderStates[i] );
    }
cout << "First founder config = ";
exploredFCStore[ GetDataWidth()-1 ][fInd].Dump();
cout << "Appending the first state: ";
DumpIntVec( exploredFCStore[ GetDataWidth()-1 ][fInd].founderStates);

    // Now we preprocess the next one by one
    int site =  GetDataWidth() - 2 ;
    while(site >= 0)
    {
cout << "LOOP: site = " << site << endl;
        // we need to find a fconfig s.t. it agrees with the min score
        int indCur = -1;
        for(unsigned int i=0; i<exploredFCStore[site].size(); ++i)
        {
//cout << "Testing a fconfig";
//exploredFCStore[site][i].Dump();
            if( IsNgbrPairCFs( exploredFCStore[site][i],  lastFConfig ) == true )
            {
                indCur = i;
                break;
            }
        }
        YW_ASSERT_INFO( indCur >=0, "Fail to find the solution" );
//cout << "Appending a state: ";
//DumpIntVec(exploredFCStore[ site ][indCur].founderStates);
        // Now save the state bit
        for(  int i=0; i<GetNumFounders(); ++i )
        {
            // Save the last bit first
            foundersBeingBuilt[i].push_back( exploredFCStore[ site ][indCur].founderStates[i] );
        }

        // Now move to the next bit
        lastFConfig =  exploredFCStore[site][indCur];

        // save it
        reversedCFs.push_back(lastFConfig);


        // move to next site
        site--;
    }


    // At last, we need to reverse the founders
    for( unsigned int i=0; i<foundersBeingBuilt.size(); ++i )
    {
        ReverseIntVec( foundersBeingBuilt[i] );
    }
    // here also reverse CFs too
    for(int i=(int)reversedCFs.size()-1; i>=0; --i)
    {
        optDFSFCs.push_back( reversedCFs[i] );
    }

    // Now set it to founders
    optDFSFounders = foundersBeingBuilt;
cout << "optDFSFounders = \n";
DumpVecSequences(optDFSFounders);

    // make sure we have enough CFs corresponding to the opt solution
    YW_ASSERT_INFO( (int)optDFSFCs.size()== GetDataWidth(), "Not enough CFs");
}

void ExactEnumFoundersAbstractSolver :: RetrieveRandFounders( 
               FOUNDER_LIST &rndFounders, vector< FounderSiteConfig > &rndDFSFCs )
{
    rndDFSFCs.clear();

    // We trace from backwards to find the minimum one on the rightmost parse
    // If there is nothing at the rightmost entry, no founders are found
    if(exploredFCStore[ GetDataWidth() -1 ].size() == 0)
    {
        cout << "Fail to find good founders3.\n";
        return;
    }
    vector<int> minList;
    GetMinBreakpoints(exploredFCStore[ GetDataWidth()-1 ], &minList);
//cout << "Last row minList = ";
//DumpIntVec( minList );
    YW_ASSERT_INFO( minList.size() > 0, "Must find at least one" );
    vector<SEQUENCE> foundersBeingBuilt;
    for(int f=0; f<GetNumFounders();++f)
    {
        SEQUENCE emptySeq;
        foundersBeingBuilt.push_back(emptySeq);
    }
    // Pick a randoe to continue
    int fInd = GetRandItemInVec( minList );
    FounderSiteConfig lastFConfig = exploredFCStore[ GetDataWidth()  - 1 ][fInd];

    // save it
    vector<FounderSiteConfig> reversedCFs;
    reversedCFs.push_back(lastFConfig);

    for(  int i=0; i<GetNumFounders(); ++i )
    {
        // Save the last bit first
        foundersBeingBuilt[i].push_back( exploredFCStore[ GetDataWidth()-1 ][fInd].founderStates[i] );
    }
//cout << "First founder config = ";
//exploredFCStore[ GetDataWidth()-1 ][fInd].Dump();
//cout << "Appending the first state: ";
//DumpIntVec( exploredFCStore[ GetDataWidth()-1 ][fInd].founderStates);

    // Now we preprocess the next one by one
    int site =  GetDataWidth() - 2 ;
    while(site >= 0)
    {
//cout << "LOOP: site = " << site << endl;
        // we need to find a fconfig s.t. it agrees with the min score
        vector<int> listChoices;
        for(unsigned int i=0; i<exploredFCStore[site].size(); ++i)
        {
//cout << "Testing a fconfig";
//exploredFCStore[site][i].Dump();
            if( IsNgbrPairCFs( exploredFCStore[site][i],  lastFConfig ) == true )
            {
                listChoices.push_back( i );
                //indCur = i;
                //break;
            }
        }
        YW_ASSERT_INFO( listChoices.size() > 0, "Fail to find the solution" );
        int indCur = GetRandItemInVec(  listChoices);
//cout << "listChoices = ";
//DumpIntVec( listChoices );

//        YW_ASSERT_INFO( indCur >=0, "Fail to find the solution" );
//cout << "Appending a state: ";
//DumpIntVec(exploredFCStore[ site ][indCur].founderStates);
        // Now save the state bit
        for(  int i=0; i<GetNumFounders(); ++i )
        {
            // Save the last bit first
            foundersBeingBuilt[i].push_back( exploredFCStore[ site ][indCur].founderStates[i] );
        }

        // Now move to the next bit
        lastFConfig =  exploredFCStore[site][indCur];

        // save it
        reversedCFs.push_back(lastFConfig);


        // move to next site
        site--;
    }


    // At last, we need to reverse the founders
    for( unsigned int i=0; i<foundersBeingBuilt.size(); ++i )
    {
        ReverseIntVec( foundersBeingBuilt[i] );
    }
    // here also reverse CFs too
    for(int i=(int)reversedCFs.size()-1; i>=0; --i)
    {
        rndDFSFCs.push_back( reversedCFs[i] );
    }

    // Now set it to founders
    rndFounders = foundersBeingBuilt;
//cout << "optDFSFounders = \n";
//DumpVecSequences(optDFSFounders);

    // make sure we have enough CFs corresponding to the opt solution
    YW_ASSERT_INFO( (int)rndDFSFCs.size()== GetDataWidth(), "Not enough CFs");
}





/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
const int EE_EXACT_WND_SIZE = 4;


ExactEnumFoundersSolver :: ExactEnumFoundersSolver( const BinaryMatrix &mat, int nF, int upperBound )
:   numFounders( nF ) , knownUpperBound(upperBound)
{
    // save the matrix
    inputMat = mat;
    // for now do not allow trimming dup rows
    //inputMat.TrimDupRows();
    for( int r=0; r<inputMat.GetRowNum(); ++r )
    {
        SEQUENCE row;
        inputMat.GetRow( r, row );
        vecRowsMultiplicity.push_back( mat.GetMultiplictyForRow( row )  );
    }
    //
 //   cout << "Input row multiplicty is: ";
 //   DumpIntVec( vecRowsMultiplicity );
}

ExactEnumFoundersSolver :: ~ExactEnumFoundersSolver()
{
}

int ExactEnumFoundersSolver :: GetMultiplicity( int r ) 
{
    // by default, return 1 to indicate no multiplicity
    //return vecRowsMultiplicity[r];  
    // for now, ignore multiplicty
    return 1;
}

// Now read in from a file
int ExactEnumFoundersSolver :: Solve()
{
    // Compute local lower bounds if needed
    // First compute local lower bounds for different intervals
    map<INTERVAL, int>  localBounds;
    int lb = ComputeBlockLB( inputMat, numFounders, &localBounds);
    YW_ASSERT_INFO( lb <= knownUpperBound, "LB/UB do not match"  );



#if 0
    // Now we try to tighten the LB by compute exact minimum when the data is large
    if( inputMat.GetColNum() >= 2*EE_EXACT_WND_SIZE  && fApproxFlag == false)
    {
        for( int ivLeft = 0; ivLeft < inputMat.GetColNum(); ++ivLeft )
        {
            int ivRight = ivLeft + EE_EXACT_WND_SIZE-1;
            if( ivRight >= inputMat.GetColNum() )
            {
                // we are done
                break;
            }
            // now take the submatrix
            BinaryMatrix submat;
            inputMat.SubMatrix( 0, inputMat.GetRowNum()-1, ivLeft, ivRight, submat  );
            ExactEnumFoundersSolver exactESolver( submat, numFounders, HAP_MAX_INT );
            exactESolver.SetDispFlag(false);
            int localMin = exactESolver.Solve();

            INTERVAL iv(ivLeft, ivRight);
cout << "Lower bound = " << localBounds[iv] << ", and ";
cout << "exact local min: left = " << ivLeft << ", right = " << ivRight << ", min = " << localMin << endl;
            YW_ASSERT_INFO( localMin < HAP_MAX_INT, "Wrong computation of local min" );

            YW_ASSERT_INFO( localBounds[iv] <= localMin, "Wrong lower bound" );
            localBounds[iv] = localMin;
        }

        // now try to enhance the global bound here....
        for( int ivLen = EE_EXACT_WND_SIZE; ivLen < inputMat.GetColNum(); ++ivLen )
        {
            for( int ivLeft = 0; ivLeft < inputMat.GetColNum(); ++ivLeft  )
            {
                int ivRight = ivLeft + ivLen - 1;
                if( ivRight >= inputMat.GetColNum() )
                {
                    // we are done
                    break;
                }

                vector<int> locBreakpoints;
                int bdNew = CalcCompositeBound( localBounds, ivLeft, ivRight, locBreakpoints );
                pair<int,int> pp(ivLeft, ivRight);
                YW_ASSERT_INFO( localBounds.find( pp ) != localBounds.end(), "FAILURE HERE: computing local bounds!" );
//cout << "Original bd = " << localBounds[pp] << ", for left=" << ivLeft << ", right=" << ivRight <<endl;
//cout << "New bd = " << bdNew << endl;
                localBounds[pp] = bdNew;
            }
        }

    }
#endif


//#if 0
    // update the global bounds
    // now try to enhance the global bound here....
    for( int ivLen = 2; ivLen < inputMat.GetColNum(); ++ivLen )
    {
        for( int ivLeft = 0; ivLeft < inputMat.GetColNum(); ++ivLeft  )
        {
            int ivRight = ivLeft + ivLen - 1;
            if( ivRight >= inputMat.GetColNum() )
            {
                // we are done
                break;
            }

            vector<int> locBreakpoints;
            int bdNew = CalcCompositeBound( localBounds, ivLeft, ivRight, locBreakpoints );
            pair<int,int> pp(ivLeft, ivRight);
            if(localBounds.find( pp ) == localBounds.end())
            {
                if( bdNew > 0 )
                {
                    localBounds.insert( map<INTERVAL, int> :: value_type( pp, bdNew )  );
                }
            }
            else
            {
            //YW_ASSERT_INFO( localBounds.find( pp ) != localBounds.end(), "FAILURE HERE: computing local bounds!" );
//cout << "Original bd = " << localBounds[pp] << ", for left=" << ivLeft << ", right=" << ivRight <<endl;
//cout << "New bd = " << bdNew << endl;
                YW_ASSERT_INFO( bdNew >=  localBounds[pp], "Bound is wrong.");
                localBounds[pp] = bdNew;
            }
        }
    }
//#endif



    // For now, just store the lower bounds
    this->ivLocalBounds = localBounds;




    // NOW, START THE REAL COMPUTATION
    // Here is what we have for founders at site 0,1,....m-1
    vector< vector<FounderSiteConfig> > listFounderSitesConfig;

    // We start by having a un-restricted founder site as start for site 0
    vector<FounderSiteConfig> site0FoundersConfig;

    // Herw are the possible founder bits
    vector<int> listFounderBits;
    for( int fb = 1; fb < numFounders; ++fb )
    {
        int founderBit = (0x1 << fb) - 1;
        listFounderBits.push_back(founderBit);
    }

    // Keep track of, for each stage, the min-breakpoints we need
    // The first site = 0
    minBkptsForSites.push_back( 0 );

    // Now we enumerate all possible founder states
    for( int i=1; i<numFounders; ++i )
    {
        // we only need to try 00001, 00011, 00111, 01111 type
        int founderBit = (0x1 << i) - 1;

        vector<int> founderState;
        GetHyperCubeSeq(founderBit, founderState, numFounders);
//cout << "Trying founder state: ";
//DumpIntVec( founderState );
        FounderSiteConfig fConfig;
        GetUnRestrictedFounderConfigAt(0, founderState, fConfig);

        // set the multiplicty here
        fConfig.InitMultiplicity(&vecRowsMultiplicity);

        // initialize the block
        fConfig.InitUnrestrictedUniBlock(founderState);

        site0FoundersConfig.push_back( fConfig );
    }
    listFounderSitesConfig.push_back( site0FoundersConfig );

    // Now start to handle the 1, 2, 3, ...., m-1 sites
    for( int site = 1; site < inputMat.GetColNum(); ++site )
    {
//cout << "Processing site = " << site << endl;
        vector<FounderSiteConfig> foundersConfigs;
        for( unsigned int i=0; i<listFounderBits.size(); ++i )
        {
            vector<int> founderState;
            GetHyperCubeSeq(listFounderBits[i], founderState, numFounders);

            FindFounderConfigs( site, founderState, listFounderSitesConfig[site-1], foundersConfigs );
        }
        if( fDispInfo == true )
        {
cout << "At site " << site << ", before prune we found configuration number = " << foundersConfigs.size() << endl;
        }

        // Initialize the multiplicty
        for( unsigned int j=0; j<foundersConfigs.size(); ++j )
        {
            foundersConfigs[j].InitMultiplicity(&vecRowsMultiplicity);
        }

        // Now prune out some no-good ones
        PruneFounderConfig(foundersConfigs);
//cout << "After prune we found configuration number = " << foundersConfigs.size() << endl;

//cout << "Found this many FCConfigs:\n";
//DumpVecFConfig(foundersConfigs );

        // Save it
        listFounderSitesConfig.push_back(foundersConfigs);

        // Also keep track of the current min-crossovers till this site
        this->minBkptsForSites.push_back( GetMinBreakpoints(foundersConfigs)  );
        if( fDispInfo == true )
        {
cout << "Current minimum = ";
DumpIntVec( this->minBkptsForSites );
        }
    }
//cout << "Now try to find the set of optimal founders..\n";
    // Save the founder
    FindSolFounders( listFounderSitesConfig );
//cout << "The founders are:\n";
    // solution,
    int res = GetMinBreakpoints( listFounderSitesConfig[inputMat.GetColNum()-1] );
    if( res == HAP_MAX_INT)
    {
        // we did not find a solution better than upper bound
        res = this->knownUpperBound;
    }
    return res;

}

void ExactEnumFoundersSolver :: GetSolFounders( FOUNDER_LIST &founders )
{
    // just return what ever we have
    founders = optFounders;
}


// This is a simple testing code
// Here we simply keep only a single path
// that has optimal solution, as what Yun Song
// brought out in the group meeting
int ExactEnumFoundersSolver :: SolveTestCode()
{
    // NOW, START THE REAL COMPUTATION
    // Here is what we have for founders at site 0,1,....m-1
    vector< vector<FounderSiteConfig> > listFounderSitesConfig;

    // We start by having a un-restricted founder site as start for site 0
    vector<FounderSiteConfig> site0FoundersConfig;

    // Herw are the possible founder bits
    // here we take all permulations to make matters simpler
    vector<int> listFounderBits;
    for( int fb = 1; fb < (0x1 << numFounders)-1; ++fb )
    {
        //int founderBit = (0x1 << fb) - 1;
        listFounderBits.push_back(fb);
    }

    // Keep track of, for each stage, the min-breakpoints we need
    // The first site = 0
    minBkptsForSites.push_back( 0 );

    // Now we enumerate all possible founder states
    for( int i=1; i<numFounders; ++i )
    {
        // we only need to try 00001, 00011, 00111, 01111 type
        int founderBit = (0x1 << i) - 1;

        vector<int> founderState;
        GetHyperCubeSeq(founderBit, founderState, numFounders);
//cout << "Trying founder state: ";
//DumpIntVec( founderState );
        FounderSiteConfig fConfig;
        GetUnRestrictedFounderConfigAt(0, founderState, fConfig);

        // initialize the block
        fConfig.InitUnrestrictedUniBlock(founderState);

        site0FoundersConfig.push_back( fConfig );
    }
    listFounderSitesConfig.push_back( site0FoundersConfig );

    // Now start to handle the 1, 2, 3, ...., m-1 sites
    // the idea is to just find one best previous founder config and save ONLY one copy
    for( int site = 1; site < inputMat.GetColNum(); ++site )
    {
        vector< vector<FounderSiteConfig> > listfConfigList;
    //cout << "Processing site = " << site << endl;
        vector<FounderSiteConfig> fConfigListStep;
        for( unsigned int i=0; i<listFounderBits.size(); ++i )
        {
            vector<int> founderState;
            GetHyperCubeSeq(listFounderBits[i], founderState, numFounders);

            int minBkptsSite = HAP_MAX_INT;
            FounderSiteConfig curBestCfg;

            for( int j=0; j<(int)listFounderSitesConfig[site-1].size(); ++j )
            {
                // build out possible config for each
                // build out possible config for each
                vector<FounderSiteConfig> fConfigList;

                // Now get the inferred min-state
                //vector<set<int> > rowsFounderPos;
//cout << "Pre-config = ";
//prevSiteFounderConfigs[i].Dump();
                BuildFounderFromPre( site, listFounderSitesConfig[site-1][j], founderState, fConfigList  );
                int nbkpt = fConfigList[0].GetMinBreakpoints();
                if( nbkpt < minBkptsSite )
                {
                    minBkptsSite = nbkpt;
                    curBestCfg = fConfigList[0];
                }

                // save it
                //for(int kkk=0; kkk<(int)fConfigList.size(); ++kkk)
                //{
                //    fConfigListStep.push_back( fConfigList[kkk] );
                //}
             }
            fConfigListStep.push_back(curBestCfg); 
        }

//cout << "After prune we found configuration number = " << foundersConfigs.size() << endl;

//cout << "Found this many FCConfigs:\n";
//DumpVecFConfig(foundersConfigs );

        listFounderSitesConfig.push_back(fConfigListStep);

        // Also keep track of the current min-crossovers till this site
        this->minBkptsForSites.push_back( GetMinBreakpoints(fConfigListStep)  );
        if( fDispInfo == true )
        {
cout << "Current minimum = ";
DumpIntVec( this->minBkptsForSites );
        }
    }
//cout << "Now try to find the set of optimal founders..\n";
    // Save the founder
    FindSolFounders( listFounderSitesConfig );
//cout << "The founders are:\n";
    // solution,
    int res = GetMinBreakpoints( listFounderSitesConfig[inputMat.GetColNum()-1] );
    if( res == HAP_MAX_INT)
    {
        // we did not find a solution better than upper bound
        res = this->knownUpperBound;
    }
    return res;

}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementaions

void ExactEnumFoundersSolver :: GetUnRestrictedFounderConfigAt(int site, const vector<int> &founderState,
                                                               FounderSiteConfig& founderConfig)
{
    YW_ASSERT_INFO( site < inputMat.GetColNum(), "Site over-reached" );
    founderConfig.site = site;;
    founderConfig.founderStates = founderState;
    YW_ASSERT_INFO( (int)founderState.size() == numFounders, "Must be equal" );
    set<int> rows0, rows1;
    for( unsigned int k=0; k<founderState.size(); ++k )
    {
        if( founderState[k] == 0  )
        {
            rows0.insert( k );
        }
        else
        {
            rows1.insert( k );
        }
    }

    // Now we retrive the states where the input rows can be at
    for( int r = 0; r<inputMat.GetRowNum(); ++r )
    {
        if( inputMat(r, site) == 0 )
        {
            founderConfig.inRowsMinPosInFounders.push_back( rows0 );
        }
        else
        {
            founderConfig.inRowsMinPosInFounders.push_back( rows1 );
        }

        // Initially there is no breakpoint
        founderConfig.numBreakpointsRows.push_back( 0 );
    }
}


void ExactEnumFoundersSolver :: BuildFounderFromPre( int site, const FounderSiteConfig &founderConfigPre, 
    const vector<int> &founderState, vector<FounderSiteConfig> &fConfigList  )
{
    fConfigList.clear();
    FounderSiteConfig fConfigCur;
    fConfigList.push_back( fConfigCur );
    fConfigList[0].site = site;
    fConfigList[0].founderStates = founderState;

    // check row bit by bit
    for( int r =0; r<inputMat.GetRowNum(); ++r )
    {
        set<int> preExitRows = founderConfigPre.inRowsMinPosInFounders[r];
        set<int> curMatchedRows;
        FindMatchedFounderRows( inputMat(r,site), founderState, curMatchedRows  );
        // Find out whether there is an intersection of current founder
        set<int> commonRows;
        JoinSets( preExitRows, curMatchedRows, commonRows );
        if( commonRows.size() == 0 )
        {
            // We start a state swtich: number of breakpoint +=1
            fConfigList[0].inRowsMinPosInFounders.push_back( curMatchedRows );
            // breakpoint number inc by 1
            fConfigList[0].numBreakpointsRows.push_back( founderConfigPre.numBreakpointsRows[r]+1 ) ;
        }
        else
        {
            fConfigList[0].inRowsMinPosInFounders.push_back( commonRows );

            // breakpoint number stay the same
            fConfigList[0].numBreakpointsRows.push_back( founderConfigPre.numBreakpointsRows[r] ) ;
        }
    }
}



bool ExactEnumFoundersSolver :: IsFounderBetterThan( const FounderSiteConfig & fConfig1, 
                                                    const FounderSiteConfig &fConfig2  ) 
{
//cout <<"IsFounderBetterThan: fConfig1 = \n";
//fConfig1.Dump();
//cout << "fConfig2 = \n";
//fConfig2.Dump(); 
    // test whether one is better than the other
    // make sure we are comparing two idnetical states
    YW_ASSERT_INFO( fConfig1.site == fConfig2.site && fConfig1.founderStates == fConfig2.founderStates, 
        "Can not compare two things that are not matched.");

    // If two things are identical, false
    if( fConfig1 == fConfig2 )
    {
        return false;
    }

    // Check to see if the crossovers are pairwise outperform
    for( unsigned int i=0; i<fConfig1.numBreakpointsRows.size(); ++i )
    {
        if( fConfig1.numBreakpointsRows[i] >  fConfig2.numBreakpointsRows[i]  )
        {
//cout << "Number of breakpoints is not better: fConfig1[i] = ";
//cout << fConfig1.numBreakpointsRows[i]  << ", fconfig2[i] = " << fConfig2.numBreakpointsRows[i]  << " i=" << i << endl;
            // no better
            return false;
        }
    }
    for(unsigned int i=0; i<fConfig1.inRowsMinPosInFounders.size(); ++i)
    {
        // If subset of rows does not contain the second, no better
        if( IsSetContainer( fConfig1.inRowsMinPosInFounders[i], fConfig2.inRowsMinPosInFounders[i] ) == false ) 
            //fConfig1.inRowsMinPosInFounders[i] != fConfig2.inRowsMinPosInFounders[i]  )
        {
//cout << "Subset " << i << " is not containined.\n";
            return false;
        }
    }
    return true;
}


bool ExactEnumFoundersSolver :: IsOptPossibleWithPartialSol( int  start, int end, int bd )
{
    // first get the minimum un-explored breakpoints
    int minRemBkpts = 0;
    INTERVAL iv(end, inputMat.GetColNum()-1);
    if( this->ivLocalBounds.find( iv) != this->ivLocalBounds.end()  )
    {
        minRemBkpts = this->ivLocalBounds[iv];
        if(minRemBkpts <= 0)
        {
            minRemBkpts = 0;
        }
    }


    // for now, we simply test for known upper bound
    if( bd + minRemBkpts < this->knownUpperBound )
    {
        return true;
    }
    else
    {
        return false;
    }


#if 0
    // Here we test whethert this is still possible to get a better solution
    map<INTERVAL,int> updatedLocalBds = this->ivLocalBounds;

    // Here we compute the min-crossovers for all segments of this known piece of founders
    // for strongest possible bound we can get
    INTERVAL iv(start, end);

    // Now retrive a fragment of founders within this IV and see if this can be better
    updatedLocalBds[iv] = bd;

    YW_ASSERT_INFO( (int)minBkptsForSites.size() > end-1, "A little too small" );
    // also update the previous segment (0,i) to the min value we know of
    for( int s=1; s<end; ++s )
    {
        INTERVAL iv(0, s);
        updatedLocalBds[iv] = minBkptsForSites[s];
    }

    // Now compute a final lower bound here
    vector<int> localBreakpoints;
    int curLB  = CalcCompositeBound( updatedLocalBds, 0, inputMat.GetColNum()-1, localBreakpoints);
    if( curLB >= this->knownUpperBound )
    {
        return false;
    }
    else
    {
        // OK, this one may lead to a good solution
        return true;
    }
#endif
}

void ExactEnumFoundersSolver :: PruneFounderConfig(vector<FounderSiteConfig> &foundersConfigs)
{
    // prune it unless it is small
    if( (int)foundersConfigs.size() < nMaxConfigNumStepToKeep )
    {
        return;
    }

    // Prune it by keeping the smallest breakpoints
    // sort it
    vector< pair<int,int> > listBkptPosPairs;
    for( int i=0; i<(int)foundersConfigs.size();++i )
    {
        pair<int,int> pp;
        pp.first = foundersConfigs[i].GetMinBreakpoints();
        pp.second = i;
        listBkptPosPairs.push_back(pp);
    }
    SortVecIntPairs(listBkptPosPairs);
    // 
    vector<FounderSiteConfig> resList;
    for( int i=0; i<nMaxConfigNumStepToKeep; ++i )
    {
        int pos = listBkptPosPairs[i].second;
        resList.push_back( foundersConfigs[pos]  );
    }
    foundersConfigs = resList;


#if 0       // does not work yet
    // Prune founder configurations
    // A simple idea: if a config uses more than numFounders more than the minimum config, then prune it
    int minBkpts = GetMinBreakpoints(foundersConfigs);
    vector<FounderSiteConfig> updatedFConfigs;
    for( unsigned int i=0; i<foundersConfigs.size(); ++i )
    {
        // We impose a cut of of ceil(numFounders/2)
        // because we can always transoform a solution by switching rows in the next site
        // to achieve such bound
        int cutoff = inputMat.GetRowNum();

        if(  foundersConfigs[i].GetMinBreakpoints() < minBkpts + cutoff  )
        {
            // This is a potentially useful one
            updatedFConfigs.push_back( foundersConfigs[i] );
        }
    }

    // What else, TBD

    // Now put it back
    foundersConfigs = updatedFConfigs;
#endif
}



void ExactEnumFoundersSolver :: PruneFounderConfigStepSwap(vector<FounderSiteConfig> &foundersConfigs)
{
#if 0
//return;

    if( foundersConfigs.size() == 0)
    {
        // If there is nothing to try, skip
        return;
    }

    // we simply normailize every config
    vector<FounderSiteConfig> updatedFConfigs;
    for( unsigned int i=0; i<foundersConfigs.size(); ++i )
    {
        FounderSiteConfig fcn;
        foundersConfigs[i].Normalize(fcn);
        updatedFConfigs.push_back( fcn );
    }

    // now prune it
    vector<int> keptPos;
    PruneFounderConfigStep( updatedFConfigs, keptPos );
    YW_ASSERT_INFO( keptPos.size() > 0, "Must keept something" );

    vector<FounderSiteConfig> keptFCs;
    for(unsigned int i=0; i<keptPos.size(); ++i)
    {
        keptFCs.push_back( foundersConfigs[ keptPos[i] ] );
    }
    foundersConfigs = keptFCs;
//exit(1);
#endif
}


void ExactEnumFoundersSolver :: FindSolFounders( const vector< vector<FounderSiteConfig> > &listFounderSitesConfig )
{
    // We trace from backwards to find the minimum one on the rightmost parse
    // If there is nothing at the rightmost entry, no founders are found
    if(listFounderSitesConfig[ inputMat.GetColNum()-1 ].size() == 0)
    {
//        cout << "Did not find solutions better than the quick-search solution.\n";
        return;
    }
    vector<int> minList;
    GetMinBreakpoints(listFounderSitesConfig[ inputMat.GetColNum()-1 ], &minList);
    YW_ASSERT_INFO( minList.size() > 0, "Must find at least one" );
    vector<SEQUENCE> foundersBeingBuilt;
    for(int f=0; f<numFounders;++f)
    {
        SEQUENCE emptySeq;
        foundersBeingBuilt.push_back(emptySeq);
    }
    // For now, pick the first one to continue
    int fInd = minList[0];
    FounderSiteConfig lastFConfig = listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd];
    for(  int i=0; i<numFounders; ++i )
    {
        // Save the last bit first
        foundersBeingBuilt[i].push_back( listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd].founderStates[i] );
    }
//cout << "First founder config = ";
//listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd].Dump();
//cout << "Appending the first state: ";
//DumpIntVec(listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd].founderStates);

    // Now we preprocess the next one by one
    int site =  inputMat.GetColNum()-2 ;
    while(site >= 0)
    {
//cout << "LOOP: site = " << site << endl;
        // we need to find a fconfig s.t. it agrees with the min score
        int indCur = -1;
        for(unsigned int i=0; i<listFounderSitesConfig[site].size(); ++i)
        {
//cout << "Testing a fconfig";
//listFounderSitesConfig[site][i].Dump();
            if( listFounderSitesConfig[site][i].IsLeftConfig(lastFConfig, inputMat) == true )
            {
                indCur = i;
                break;
            }
        }
        YW_ASSERT_INFO( indCur >=0, "Fail to find the solution" );
//cout << "Appending a state: ";
//DumpIntVec(listFounderSitesConfig[ site ][indCur].founderStates);
        // Now save the state bit
        for(  int i=0; i<numFounders; ++i )
        {
            // Save the last bit first
            foundersBeingBuilt[i].push_back( listFounderSitesConfig[ site ][indCur].founderStates[i] );
        }

        // Now move to the next bit
        lastFConfig =  listFounderSitesConfig[site][indCur];

        // move to next site
        site--;
    }


    // At last, we need to reverse the founders
    for( unsigned int i=0; i<foundersBeingBuilt.size(); ++i )
    {
        ReverseIntVec( foundersBeingBuilt[i] );
    }
    // Now set it to founders
    optFounders = foundersBeingBuilt;
    optDFSFounders = foundersBeingBuilt;
//cout << "optDFSFounders = \n";
//DumpVecSequences(optDFSFounders);

}

int ExactEnumFoundersSolver :: GetDataWidth() 
{
    return this->inputMat.GetColNum();
}

bool ExactEnumFoundersSolver :: IsNgbrPairCFs(const FounderSiteConfig &leftCF, const FounderSiteConfig &rightCF ) 
{
    return  leftCF.IsLeftConfig(rightCF, inputMat );

}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//  
//  Genotype block solver here
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ExactEnumGenoFoundersSolver :: ExactEnumGenoFoundersSolver( const GenotypeMatrix &mat, int nF )
    : inputMat( mat ), numFounders( nF ), knownUpperBound(HAP_MAX_INT), fReductionEnabled( false )
{
    if( fReductionEnabled == true )
    {
        ReduceData();
    }
    else
    {
        // Otherwise, the reduced data is the same as the original
        reducedGenoMat = inputMat;

        // Now we remove uniform sites from matrix
        set<int> uniSites;
        for( int c=0; c<inputMat.GetColNum(); ++c )
        {
            int nc = inputMat.GetNumZerosAtSite( c );
            if( nc == 0 || nc == inputMat.GetRowNum() * 2 )
            {
                // This is a uniform site
                uniSites.insert( c );
            }
        }
        // now remove these sites
        reducedGenoMat.RemoveColumns( uniSites );
    }

    // find out what is the position of the first 2s for each row
    for(int r=0; r<reducedGenoMat.GetRowNum(); ++r )
    {
        int pos = -1;
        for(int c = 0; c< reducedGenoMat.GetColNum(); ++c)
        {
            if( reducedGenoMat(r, c) == 2 )
            {
                pos = c;
                break;
            }
        }
        posFirst2s.push_back(pos);
    }

    // Here we compute a lower bound first
    int lb = ComputeBlockGenoLB( reducedGenoMat, numFounders, &mapIVLBs );
    cout << "The lower bound for minimum breakpoints = " << lb << endl;

}

ExactEnumGenoFoundersSolver :: ~ExactEnumGenoFoundersSolver()
{
}


// Now read in from a file
int ExactEnumGenoFoundersSolver::  Solve()
{
    // Here is what we have for founders at site 0,1,....m-1
    vector< vector<FounderSiteConfig> > listFounderSitesConfig;

    // We start by having a un-restricted founder site as start for site 0
    vector<FounderSiteConfig> site0FoundersConfig;

    // Herw are the possible founder bits
    vector<int> listFounderBits;
    for( int fb = 1; fb < numFounders; ++fb )
    {
        int founderBit = (0x1 << fb) - 1;
        listFounderBits.push_back(founderBit);
    }

    // Keep track of, for each stage, the min-breakpoints we need
    // The first site = 0
    minBkptsForSites.push_back( 0 );

    // Now we enumerate all possible founder states
    for( int i=1; i<numFounders; ++i )
    {
        // we only need to try 00001, 00011, 00111, 01111 type
        int founderBit = (0x1 << i) - 1;

        vector<int> founderState;
        GetHyperCubeSeq(founderBit, founderState, numFounders);
cout << "Trying founder state: ";
DumpIntVec( founderState );
        FounderSiteConfig fConfig;
        GetUnRestrictedFounderConfigAt(0, founderState, fConfig);

        site0FoundersConfig.push_back( fConfig );
    }
    listFounderSitesConfig.push_back( site0FoundersConfig );

    // Now start to handle the 1, 2, 3, ...., m-1 sites
    for( int site = 1; site < reducedGenoMat.GetColNum(); ++site )
    {
cout << "Processing site = " << site << endl;
        vector<FounderSiteConfig> foundersConfigs;
        for( unsigned int i=0; i<listFounderBits.size(); ++i )
        {
            vector<int> founderState;
            GetHyperCubeSeq(listFounderBits[i], founderState, numFounders);

            FindFounderConfigs( site, founderState, listFounderSitesConfig[site-1], foundersConfigs );
        }
        if( fDispInfo == true )
        {
cout << "At site " << site << ", before prune we found configuration number = " << foundersConfigs.size() << endl;
        }

        // Now prune out some no-good ones
//        PruneFounderConfig(foundersConfigs);
//cout << "After prune we found configuration number = " << foundersConfigs.size() << endl;

//cout << "Found this many FCConfigs:\n";
//DumpVecFConfig(foundersConfigs );

        // Save it
        listFounderSitesConfig.push_back(foundersConfigs);

        // Also keep track of the current min-crossovers till this site
        this->minBkptsForSites.push_back( GetMinBreakpoints(foundersConfigs)  );
        if( fDispInfo == true )
        {
cout << "Current minimum = ";
DumpIntVec( this->minBkptsForSites );
        }
    }

    // Save the founder
//    BuildDFSSolFounders(  );
    FindSolFounders( listFounderSitesConfig );

    // also build the entries

    // solution,
    int res = GetMinBreakpoints( listFounderSitesConfig[reducedGenoMat.GetColNum()-1] );
    if( res == HAP_MAX_INT)
    {
        // we did not find a solution better than upper bound
        res = this->knownUpperBound;
    }
    return res;

}

void ExactEnumGenoFoundersSolver :: GetPhasedMatrix( BinaryMatrix &res, FOUNDER_LIST &founders )
{
    // we first phase according to the result
    //ParseInputMat( optDFSFCs  );
    founders = optDFSFounders;
    res = phasedMat;
}

void ExactEnumGenoFoundersSolver :: FindSolFounders( const vector< vector<FounderSiteConfig> > &listFounderSitesConfig)
{
    // set phased matrix and save the founders from the config
    // We trace from backwards to find the minimum one on the rightmost parse
    // If there is nothing at the rightmost entry, no founders are found
    if(listFounderSitesConfig[ inputMat.GetColNum()-1 ].size() == 0)
    {
//        cout << "Did not find solutions better than the quick-search solution.\n";
        return;
    }
    vector<int> minList;
    GetMinBreakpoints(listFounderSitesConfig[ inputMat.GetColNum()-1 ], &minList);
    YW_ASSERT_INFO( minList.size() > 0, "Must find at least one" );
    vector<SEQUENCE> foundersBeingBuilt;
    for(int f=0; f<numFounders;++f)
    {
        SEQUENCE emptySeq;
        foundersBeingBuilt.push_back(emptySeq);
    }
    // For now, pick the first one to continue
    vector<FounderSiteConfig> listOptFounderConfigs( inputMat.GetColNum() );

    // start searching
    int fInd = minList[0];
    FounderSiteConfig lastFConfig = listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd];
    listOptFounderConfigs[inputMat.GetColNum()-1] =  lastFConfig ;
    for(  int i=0; i<numFounders; ++i )
    {
        // Save the last bit first
        foundersBeingBuilt[i].push_back( listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd].founderStates[i] );
    }
//cout << "First founder config = ";
//listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd].Dump();
//cout << "Appending the first state: ";
//DumpIntVec(listFounderSitesConfig[ inputMat.GetColNum()-1 ][fInd].founderStates);

    // Now we preprocess the next one by one
    int site =  inputMat.GetColNum()-2 ;
    while(site >= 0)
    {
//cout << "LOOP: site = " << site << endl;
        // we need to find a fconfig s.t. it agrees with the min score
        int indCur = -1;
        for(unsigned int i=0; i<listFounderSitesConfig[site].size(); ++i)
        {
//cout << "Testing a fconfig";
//listFounderSitesConfig[site][i].Dump();
            vector<bool> fZeroFlags;
            if( listFounderSitesConfig[site][i].IsLeftConfig(lastFConfig, inputMat, fZeroFlags) == true )
            {
                indCur = i;
                break;
            }
        }
        YW_ASSERT_INFO( indCur >=0, "Fail to find the solution" );
//cout << "Appending a state: ";
//DumpIntVec(listFounderSitesConfig[ site ][indCur].founderStates);
        // Now save the state bit
        for(  int i=0; i<numFounders; ++i )
        {
            // Save the last bit first
            foundersBeingBuilt[i].push_back( listFounderSitesConfig[ site ][indCur].founderStates[i] );
        }

        // Now move to the next bit
        lastFConfig =  listFounderSitesConfig[site][indCur];
        listOptFounderConfigs[site] =  lastFConfig ;

        // move to next site
        site--;
    }


    // At last, we need to reverse the founders
    for( unsigned int i=0; i<foundersBeingBuilt.size(); ++i )
    {
        ReverseIntVec( foundersBeingBuilt[i] );
    }
    // Now set it to founders
    optFounders = foundersBeingBuilt;
    optDFSFounders = foundersBeingBuilt;
cout << "optDFSFounders = \n";
DumpVecSequences(optDFSFounders);


    // now phase the genotypes
    // Now fill in the matrix
    phasedMat.SetSize( reducedGenoMat.GetRowNum()*2, reducedGenoMat.GetColNum()  );

    // Start to fill site by site
    // the first site is trivially filled
    for( int c=0; c<reducedGenoMat.GetColNum(); ++c )
    {
        vector<bool> phasedway;
        if( c>= 1 )
        {
            bool f = listOptFounderConfigs[c-1].IsLeftConfig(listOptFounderConfigs[c], reducedGenoMat, phasedway);
            YW_ASSERT_INFO( f== true, "Fail to do it here.");
            YW_ASSERT_INFO( (int)phasedway.size() == reducedGenoMat.GetRowNum(), "Here is wrong" );
        }

        for( int r = 0; r< reducedGenoMat.GetRowNum(); ++r )
        {
            if( reducedGenoMat(r, c ) != 2  )
            {
                phasedMat(2*r, c) = reducedGenoMat(r, c ) ;
                phasedMat(2*r+1, c) = reducedGenoMat(r, c ) ;
            }
            else
            {
                // For the 2s, check to see if this the first 2. if so, fill trivially
                if( posFirst2s[r] == c )
                {
                    phasedMat(2*r, c) = 0;
                    phasedMat(2*r+1, c) = 1 ;
                }
                else
                {
                    // Now we need to resoret to the founder config
                    if( phasedway[r] == true )
                    {
                        phasedMat(2*r, c) = 0;
                        phasedMat(2*r+1, c) = 1 ;
                    }
                    else
                    {
                        phasedMat(2*r, c) = 1;
                        phasedMat(2*r+1, c) = 0 ;
                    }

                }
            }
        }
    }
}

const int TEST_ADDITIONAL_SOLUTION_NUM = 50;

void ExactEnumGenoFoundersSolver :: GetDFSPhasedMatrix( BinaryMatrix &res, FOUNDER_LIST &founders )
{
    // we first phase according to the result
    ParseInputMat( optDFSFCs  );
    founders = optDFSFounders;
    res = phasedMat;

#if 0
    int parsRowNum = phasedMat.GetNodupRowsNum();
cout << "Init solution = ";
res.Dump();
cout << "Init unique row num = " << parsRowNum << endl;


    // Now we start to try a small number of OPT solutions to find the most parsimonious solution
    for(int t = 0; t<TEST_ADDITIONAL_SOLUTION_NUM; ++t)
    {
        // Get a randome opt solution
        vector<FounderSiteConfig> otherOptDFSFCs;
        FOUNDER_LIST otherOptDFSFounders;
        RetrieveRandFounders( otherOptDFSFounders, otherOptDFSFCs );

        // Now try to parse it
        // but first clear hte matrix
        phasedMat.Clear();
        ParseInputMat( otherOptDFSFCs );
cout << "Additional solution matrix = ";
phasedMat.Dump();
        int stepPars = phasedMat.GetNodupRowsNum();
cout << "This solution has num of rows = " << stepPars << endl;
        if( stepPars < parsRowNum )
        {
cout << "This solution is better.\n";
           parsRowNum = stepPars;
           res = phasedMat;
           founders = otherOptDFSFounders;
        }
    }
#endif


#if 0
    // Now we add back uniform sites, if needed
    // Now we remove uniform sites from matrix
    vector<int> uniSites;
    vector<SEQUENCE> uniSitesVals;
    for( int c=0; c<inputMat.GetColNum(); ++c )
    {
        int nc = inputMat.GetNumZerosAtSite( c );
        if( nc == 0 || nc == inputMat.GetRowNum() * 2 )
        {
            // This is a uniform site
            uniSites.push_back( c );

            SEQUENCE useq;
            for( int r=0; r<res.GetRowNum(); ++r )
            {
                if( nc == 0  )
                {
                    useq.push_back( 1 );
                }
                else
                {
                    useq.push_back( 0 );
                }
            }
            uniSitesVals.push_back( useq );
        }
    }    
    if( uniSites.size() > 0 )
    {
        res.InsertColumns( uniSitesVals, uniSites  );
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation functions

void ExactEnumGenoFoundersSolver :: GetUnRestrictedFounderConfigAt(int site, const vector<int> &founderState,
                                                               FounderSiteConfig& founderConfig)
{
//cout << "GetUnRestrictedFounderConfigAt : \n";
    YW_ASSERT_INFO( site < reducedGenoMat.GetColNum(), "Site over-reached" );
    founderConfig.site = site;;
    founderConfig.founderStates = founderState;
    YW_ASSERT_INFO( (int)founderState.size() == numFounders, "Must be equal" );
    set<int> rows0, rows1;
    for( unsigned int k=0; k<founderState.size(); ++k )
    {
        if( founderState[k] == 0  )
        {
            rows0.insert( k );
        }
        else
        {
            rows1.insert( k );
        }
    }

    // Now we retrive the states where the input rows can be at
    // Note that we have twice as many rows as reducedGenoMat
    for( int r = 0; r<reducedGenoMat.GetRowNum(); ++r )
    {
        if( reducedGenoMat(r, site) == 0 )
        {
            // we have two rows
            founderConfig.inRowsMinPosInFounders.push_back( rows0 );
            founderConfig.inRowsMinPosInFounders.push_back( rows0 );
        }
        else if( reducedGenoMat(r, site) == 1  )
        {
            founderConfig.inRowsMinPosInFounders.push_back( rows1 );
            founderConfig.inRowsMinPosInFounders.push_back( rows1 );
        }
        else
        {
            // This is a heterozygote, for a unrestricted site
            // we make the top row a 0 and bottom row a 1 (for now)
            founderConfig.inRowsMinPosInFounders.push_back( rows0 );
            founderConfig.inRowsMinPosInFounders.push_back( rows1 );
        }

        // Initially there is no breakpoint for both rows
        founderConfig.numBreakpointsRows.push_back( 0 );
        founderConfig.numBreakpointsRows.push_back( 0 );
    }
}


void ExactEnumGenoFoundersSolver :: BuildFounderFromPre( int site, const FounderSiteConfig &founderConfigPre, 
    const vector<int> &founderState, vector<FounderSiteConfig> &fConfigList  ) 
{
//cout << "BuildFounderFromPre entry...\n";
//cout << "founderConfigPre = ";
//founderConfigPre.Dump();

    // find out how many 2s in this site
    vector<int> twoRows;
    map<int, int> fixed2s;
    for( int r =0; r<reducedGenoMat.GetRowNum(); ++r )
    {
        if( reducedGenoMat(r, site) == 2 )
        {
            if( site !=   posFirst2s[r]  )
            {
                int quickcode = QuickTest2(r, site, founderConfigPre, founderState);
                if(quickcode == 2)
                {
                    twoRows.push_back( r );
                }
                else
                {
//cout << "This 2 is fixed at " << r << ", site = " << site << endl;
                    fixed2s.insert( map<int,int> :: value_type(r, quickcode )  );
                }
            }
            else
            {
                // This is the 2s whose phase is fixed because it is the first 
                fixed2s.insert( map<int,int> :: value_type(r, 0 )  );
            }
        }
    }
//cout << "site = " << site << ", twoRows = ";
//DumpIntVec( twoRows );

    // Now we simply enumerate all possible phasing of the rows and
    // for each phasing, we genrate a configuration
    fConfigList.clear();

    for(int ss = 0; ss<=(int)twoRows.size(); ++ss)
    {
        // Now try to take all subsets of rows
        vector<int> posvec;
        GetFirstCombo( ss, twoRows.size(), posvec );

        while( true )
        {
            set<int> twosPicked;
            for( unsigned int iii = 0; iii < posvec.size(); ++iii )
            {
                twosPicked.insert( twoRows[  posvec[iii]  ]  );
            }

            //PopulateSetByVec(twosPicked, posvec);
//cout << "twosPicked = ";
//DumpIntSet( twosPicked );

            // genrate one config here
            FounderSiteConfig fConfigCur;
            fConfigList.push_back( fConfigCur );
            int curInd = (int)fConfigList.size() - 1;
            fConfigList[curInd].site = site;
            fConfigList[curInd].founderStates = founderState;

            // check row bit by bit
            for( int r =0; r<reducedGenoMat.GetRowNum(); ++r )
            {
                // Is this cell a 2?
                int phase2[2];
                if( reducedGenoMat(r, site) == 0  )
                {
                    phase2[0] = 0;
                    phase2[1] = 0;
                }
                else if( reducedGenoMat(r, site) == 1 )
                {
                    phase2[0] = 1;
                    phase2[1] = 1;
                }
                else
                {
                    // heterozygote. If this is prefixed then ...
                    if( fixed2s.find(r) != fixed2s.end()  )
                    {
                        // Either this is prefixed 2s or it is not one of the piced twos
                        phase2[0] = fixed2s[r];
                        phase2[1] = 1-fixed2s[r];
                    }
                    else if (twosPicked.find(r) ==  twosPicked.end() )
                    {
                        phase2[0] = 0;
                        phase2[1] = 1;
                    }
                    else
                    {
                        phase2[0] = 1;
                        phase2[1] = 0;

                    }
                }
//cout << "phase2[0] = " << phase2[0] << ", phase2[1] = " << phase2[1] << endl;

                // now handle the two rows corresponding to this genotype
                for(int gg = 0; gg <2; ++gg )
                {
                    int rowghap = 2*r + gg;
                    set<int> preExitRows = founderConfigPre.inRowsMinPosInFounders[rowghap];
                    set<int> curMatchedRows;
                    FindMatchedFounderRows( phase2[gg], founderState, curMatchedRows  );
                    // Find out whether there is an intersection of current founder
                    set<int> commonRows;
                    JoinSets( preExitRows, curMatchedRows, commonRows );
                    if( commonRows.size() == 0 )
                    {
                        // We start a state swtich: number of breakpoint +=1
                        fConfigList[curInd].inRowsMinPosInFounders.push_back( curMatchedRows );
                        // breakpoint number inc by 1
                        fConfigList[curInd].numBreakpointsRows.push_back( founderConfigPre.numBreakpointsRows[rowghap]+1 ) ;
                    }
                    else
                    {
                        fConfigList[curInd].inRowsMinPosInFounders.push_back( commonRows );

                        // breakpoint number stay the same
                        fConfigList[curInd].numBreakpointsRows.push_back( founderConfigPre.numBreakpointsRows[rowghap] ) ;
                    }
                }
            }
//cout << "Generate one config:";
//fConfigList[curInd].Dump();
            // get next combo
            if( GetNextCombo( ss, twoRows.size(), posvec ) == false )
            {
                break;
            }
        }
    }
//cout << "fConfigList.size() = " << fConfigList.size() << endl;
}

void ExactEnumGenoFoundersSolver :: PruneFounderConfig(vector<FounderSiteConfig> &foundersConfigs) 
{
}

void ExactEnumGenoFoundersSolver :: PruneFounderConfigStepSwap(vector<FounderSiteConfig> &foundersConfigs) 
{
}

//void ExactEnumGenoFoundersSolver :: PruneFounderConfigStep(vector<FounderSiteConfig> &foundersConfigs, 
//                                                           vector<int> &keptPos) 
//{
    // do nothing for now
//    ExactEnumFoundersAbstractSolver::PruneFounderConfigStep(foundersConfigs, keptPos);
//}

void ExactEnumGenoFoundersSolver :: PruneFounderConfigStepPerm(vector<FounderSiteConfig> &foundersConfigs) 
{
    // do nothing for now
    ExactEnumFoundersAbstractSolver::PruneFounderConfigStepPerm(foundersConfigs);
}


bool ExactEnumGenoFoundersSolver :: IsOptPossibleWithPartialSol( int  start, int end, int bd )
{
    // first get the minimum un-explored breakpoints
    int minRemBkpts = 0;
    INTERVAL iv(end, reducedGenoMat.GetColNum()-1);
    if( mapIVLBs.find( iv) != mapIVLBs.end()  )
    {
        minRemBkpts = mapIVLBs[iv];
        if(minRemBkpts <= 0)
        {
            minRemBkpts = 0;
        }
    }


    // for now, we simply test for known upper bound
    if( bd + minRemBkpts < this->knownUpperBound )
    {
        return true;
    }
    else
    {
        return false;
    }
}

int ExactEnumGenoFoundersSolver :: QuickTest2( int r, int site, const FounderSiteConfig& preFC, 
                                              const vector<int> &founderState )
{
//cout << "site = " << site << ", r = " << r << endl;
//cout << "preFC = ";
//preFC.Dump();
//cout << "founder state = ";
//DumpIntVec( founderState );
    // This function return 0/1/2
    // 0/1 means phase the top as 0/1, 2 means non-determinstistic
    
    // make sure this position is a 2
    YW_ASSERT_INFO( reducedGenoMat(r, site) == 2, "Wrong funcgtion" );

    // create 2 configs
    FounderSiteConfig fcCmps[2];

    int phase2[2][2] = 
    {
        {0, 1},
        {1, 0}
    };

    // now handle the two rows corresponding to this genotype
    for(int ff = 0; ff<2; ++ff)
    {
        for(int gg = 0; gg <2; ++gg )
        {
            int rowghap = 2*r + gg;
            set<int> preExitRows = preFC.inRowsMinPosInFounders[rowghap];
            set<int> curMatchedRows;
            FindMatchedFounderRows( phase2[ff][gg], founderState, curMatchedRows  );
            // Find out whether there is an intersection of current founder
            set<int> commonRows;
            JoinSets( preExitRows, curMatchedRows, commonRows );
            if( commonRows.size() == 0 )
            {
                // We start a state swtich: number of breakpoint +=1
                fcCmps[ff].inRowsMinPosInFounders.push_back( curMatchedRows );
                // breakpoint number inc by 1
                fcCmps[ff].numBreakpointsRows.push_back( preFC.numBreakpointsRows[rowghap]+1 ) ;
            }
            else
            {
                fcCmps[ff].inRowsMinPosInFounders.push_back( commonRows );

                // breakpoint number stay the same
                fcCmps[ff].numBreakpointsRows.push_back( preFC.numBreakpointsRows[rowghap] ) ;
            }
        }
    }
    // now swap the second config here
    set<int>  tmpRowins = fcCmps[1].inRowsMinPosInFounders[1];
    int tmpBkpts = fcCmps[1].numBreakpointsRows[1];
    fcCmps[1].inRowsMinPosInFounders[1] = fcCmps[1].inRowsMinPosInFounders[0];
    fcCmps[1].inRowsMinPosInFounders[0] = tmpRowins;
    fcCmps[1].numBreakpointsRows[1] = fcCmps[1].numBreakpointsRows[0];
    fcCmps[1].numBreakpointsRows[0] = tmpBkpts;

//cout << "QuickTest2: ";
//fcCmps[0].Dump();
//fcCmps[1].Dump();
//exit(1);

    if( fcCmps[0].CalcRelativeScore( fcCmps[1] )  <= 0 )
    {
//cout << "2 is better.\n";
//exit(1);
        // first is better
        return 1;
    }
    if( fcCmps[1].CalcRelativeScore( fcCmps[0] )  <= 0 )
    {
//cout << "1 is better.\n";
//exit(1);
        return 0;
    }
    else
    {
//cout << "Ambigouus.\n";
//exit(1);
        // not deterministic
        return 2;
    }

}

int ExactEnumGenoFoundersSolver :: GetDataWidth() 
{
    return this->reducedGenoMat.GetColNum();
}

bool ExactEnumGenoFoundersSolver :: IsNgbrPairCFs(const FounderSiteConfig &leftCF, const FounderSiteConfig &rightCF ) 
{
    vector<bool> notused;
    return  leftCF.IsLeftConfig(rightCF, reducedGenoMat, notused);
//   return  leftCF.IsLeftConfig(rightCF, reducedGenoMat);

}

void ExactEnumGenoFoundersSolver :: ParseInputMat(const vector<FounderSiteConfig> &vecOfCFs)
{
    // Now fill in the matrix
    phasedMat.SetSize( reducedGenoMat.GetRowNum()*2, reducedGenoMat.GetColNum()  );

    // Start to fill site by site
    // the first site is trivially filled
    for( int c=0; c<reducedGenoMat.GetColNum(); ++c )
    {
        vector<bool> phasedway;
        if( c>= 1 )
        {
            bool f = vecOfCFs[c-1].IsLeftConfig(vecOfCFs[c], reducedGenoMat, phasedway);
            YW_ASSERT_INFO( f== true, "Fail to do it here.");
            YW_ASSERT_INFO( (int)phasedway.size() == reducedGenoMat.GetRowNum(), "Here is wrong" );
        }

        for( int r = 0; r< reducedGenoMat.GetRowNum(); ++r )
        {
            if( reducedGenoMat(r, c ) != 2  )
            {
                phasedMat(2*r, c) = reducedGenoMat(r, c ) ;
                phasedMat(2*r+1, c) = reducedGenoMat(r, c ) ;
            }
            else
            {
                // For the 2s, check to see if this the first 2. if so, fill trivially
                if( posFirst2s[r] == c )
                {
                    phasedMat(2*r, c) = 0;
                    phasedMat(2*r+1, c) = 1 ;
                }
                else
                {
                    // Now we need to resoret to the founder config
                    if( phasedway[r] == true )
                    {
                        phasedMat(2*r, c) = 0;
                        phasedMat(2*r+1, c) = 1 ;
                    }
                    else
                    {
                        phasedMat(2*r, c) = 1;
                        phasedMat(2*r+1, c) = 0 ;
                    }

                }
            }
        }
    }
}




void ExactEnumGenoFoundersSolver :: ReduceData()
{
//    const double FRACT_MUT_CUTOFF = 0.2;

    // we drop any site that is clearly singleton
    set<int> trivSites;
    for(int c=0; c<inputMat.GetColNum(); ++c)
    {
        if( inputMat.IsSiteTrival( c ) == true )
        {
            trivSites.insert(c);
        }
    }

    // Now we start to consider whether we can infer the recent mutations
    set< int > listPotMutSites;
    for(int c=0; c<inputMat.GetColNum(); ++c)
    {
        // do not consider trivial ones
        if( trivSites.find( c ) != trivSites.end() )
        {
            continue;
        }

        int numZeros = inputMat.GetNumZerosAtSite( c );
        int numOnes = inputMat.GetRowNum()*2 - numZeros;
        int nMinor = numZeros;
        int mState = 0;
        if( nMinor > numOnes )
        {
            mState = 1;
            nMinor = numOnes;
        }

        // Is this site a good fit? For now, we only consider a site with two 1s
        if( nMinor == 2     )
        {
            //pair<int,int> p(mState, c);
            listPotMutSites.insert( c );
        }
    }

    // Now we consider site by site, where we can fix the choices
    set<int> listSitesRed = trivSites;
    for( set<int> :: iterator it =listPotMutSites.begin(); it != listPotMutSites.end(); ++it  )
    {
        // Now see if we can determine it
        if( IsSiteRecentMut( *it   )  == true  )
        {
            listSitesRed.insert( *it );
        }
    }

    // Now we reduce the matrix
    set<int> remSites;
    PopulateSetWithInterval(remSites, 0, inputMat.GetColNum()-1);
    SubtractSets( remSites,  listSitesRed);
    vector<int> vecRemSites;
    PopulateVecBySet( vecRemSites, remSites );
    inputMat.SubMatrixSelectedSites( vecRemSites, reducedGenoMat );
}

void ExactEnumGenoFoundersSolver :: FinishSolData(  const BinaryMatrix &partialResult   )
{
    // TBD
}

bool ExactEnumGenoFoundersSolver :: IsSiteRecentMut(int site )
{
    // Test whether we can decide this site is a recent mutatant by combining the two sites into one single row
    int numZeros = inputMat.GetNumZerosAtSite( site );
    int numOnes = inputMat.GetRowNum()*2 - numZeros;
    int nMinor = numZeros;
    int mState = 0;
    if( nMinor > numOnes )
    {
        mState = 1;
        nMinor = numOnes;
    }
    YW_ASSERT_INFO( nMinor == 2, "Minor number is wrong." );

    vector<int> stateRows;
    inputMat.GetStateRowsAtSite( site, mState, stateRows );
    YW_ASSERT_INFO( stateRows.size() == 1 || stateRows.size() == 2, "Staterows is wrong." );

    if( stateRows.size() == 1 )
    {
        // Then this row should be homogenious, no twos
        if( inputMat.GetNumTwosInRow( stateRows[0]   ) == 0  )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // In this case, we test if we can make the two genotype rows phased
        // s.t. the two haplotypes with the minor alleles are exactly identical

        return false;
    }
}


