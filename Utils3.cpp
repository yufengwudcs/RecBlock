
#include "Utils3.h"
#include "ctime"
#include "cstdlib"
#include "cstdio"


/////////////////////////////////////////////////////////////////////////////////////////
// HashTable functions
/////////////////////////////////////////////////////////////////////////////////////////

YWHashItem :: ~YWHashItem()
{
}


YWHashTable :: YWHashTable( int nB ) : numBuckets( nB)
{
}

YWHashTable :: ~YWHashTable()
{
    // NOTE: has to free memory here
    for( unsigned int i=0; i<hashTable.size(); ++i)
    {
        delete hashTable[i];
    }
    hashTable.clear();
}


void YWHashTable :: AddItem( YWHashItem *pItem )
{
    hashTable.push_back( pItem );
}

YWHashItem* YWHashTable :: GetIdenticalItem( YWHashItem *pItem )
{
cout << "GetIdenticalItem: key = " << pItem->Key() << endl;
    for( unsigned int i=0; i<hashTable.size(); ++i)
    {
//cout << "We are here.\n";
        YW_ASSERT_INFO( hashTable[i] != NULL, "Can not be nothing here." );
        if( *hashTable[i]  == *pItem )
        {
cout << "find it here.\n";
            return hashTable[i];
        }
    }
cout << "did not find.\n";
    return NULL;
}

YWHashItem *  YWHashTable :: GetFirstItem()
{
cout << "GetFirstItem: size = " << hashTable.size() << endl;
    if( hashTable.size() == 0 )
    {
        return NULL;
    }
    this->curPos = 0;
    return hashTable[0];
}

YWHashItem * YWHashTable :: GetNextItem()
{
cout << "GetNextItem: size = " << hashTable.size() ;
cout << ", curPos = " << curPos << endl;
    if( this->curPos + 1>= (int)hashTable.size() )
    {
cout << "No more item.\n";
        return NULL;
    }
    this->curPos ++;
    YWHashItem *pItem = hashTable[ this->curPos ];
    YW_ASSERT_INFO( pItem != NULL, "Can not be nothing." );
cout << "GetNextItem.key() = " << pItem->Key() << endl;
    return pItem;
}

int YWHashTable :: GetTotalItemNum() const
{
    return hashTable.size();
}

void YWHashTable :: Dump() const
{
    for( unsigned int i=0; i<hashTable.size(); ++i)
    {
//cout << "We are here.\n";
        cout << "Key for item " << i << " = " << hashTable[i]->Key() << endl;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
// substring functions
/////////////////////////////////////////////////////////////////////////////////////////
bool IsIntervalContained( const INTERVAL &iv1, const INTERVAL &iv2)
{
    if( (iv1.first  >= iv2.first && iv1.second <= iv2.second) 
        ||  (iv2.first  >= iv1.first && iv2.second <= iv1.second)     )
    {
        return true;
    }
    return false;
}
int GetIntervalLen( const INTERVAL &iv)
{
    return iv.second - iv.first + 1;
}

int GetRandItemInSet( const set<int> &items )
{
    vector<int> itemsVec;
    PopulateVecBySet( itemsVec, items );
    return GetRandItemInVec(itemsVec);
}

int GetRandItemInVec( const vector<int> &items )
{
    YW_ASSERT_INFO( items.size() >0, "You can not sample from an empty set" );

    double frac = GetRandFraction();
    return items[ (int)(items.size() * frac) ];
}

int GetWeightedRandItemInVec( const vector<int> &items, const vector<double> &itemWeights )
{
//cout << "items = ";
//DumpIntVec( items );

    YW_ASSERT_INFO( items.size() == itemWeights.size(), "Size mismatch" );
    double accum = 0.0;
    for( unsigned int i=0; i<itemWeights.size(); ++i )
    {
//cout << "one weight = " << itemWeights[i] << endl;
        accum += itemWeights[i];
    }
    YW_ASSERT_INFO( accum > 0.0000001, "Can not be too small" );
    double frac = GetRandFraction();
    double curFract = 0.0;
    for( unsigned int i=0; i<itemWeights.size(); ++i )
    {
        curFract += itemWeights[i]/accum;
        if( curFract >= frac )
        {
            return items[i];
        }
    }
    return -1;      // should nothappen


}

// This functionreturn a weighted uniformly item index from the list
int GetWeightedRandItemIndex( const vector<double> &itemWeights )
{
    double accum = 0.0;
    for( unsigned int i=0; i<itemWeights.size(); ++i )
    {
//cout << "one weight = " << itemWeights[i] << endl;
        accum += itemWeights[i];
    }
    YW_ASSERT_INFO( accum > 0.0000001, "Can not be too small" );
    double frac = GetRandFraction();
    double curFract = 0.0;
    for( unsigned int i=0; i<itemWeights.size(); ++i )
    {
        curFract += itemWeights[i]/accum;
        if( curFract >= frac )
        {
            return i;
        }
    }
    // Can not come here
    YW_ASSERT_INFO(false, "Something wrong here");
    return -1;      // should nothappen

}

void GetOrigSubset( const vector<int> &origVec, const set<int> &subsetInd, set<int> &subsetOrig )
{
    subsetOrig.clear();
    for( set<int> :: iterator it = subsetInd.begin(); it != subsetInd.end(); ++it )
    {
        YW_ASSERT_INFO( *it < (int)origVec.size(), "Size exceeds" );
        subsetOrig.insert( origVec[*it] );
    }
}

void MutateSequenceAtSites( SEQUENCE & mutSeq, vector<int> & mutSites )
{
    for( unsigned int p=0; p<mutSites.size(); ++p )
    {
        MutateSeqAtSite( mutSeq, mutSites[p]  );
    }
}

void DumpDoubleVec(const vector<double> &vecDoubles)
{
    cout << "Double vector contains: ";
    for( unsigned int i=0; i<vecDoubles.size(); ++i)
    {
        cout << vecDoubles[i] << ", ";
    }
    cout << endl;
}


bool GetFirstMutliChoice( int numStage, int numStageElem, vector<int> &initChoice )
{
    if( numStage <= 0 || numStageElem <= 0 )
    {
        return false;
    }
    initChoice.clear();
    // Start by picking first one each time
    for( int i=0; i<numStage; ++i )
    {
        initChoice.push_back( 0 );
    }
    return true;
}

bool GetNextMutliChoice( int numStage, int numStageElem, vector<int> &indChoice )
{
    // Now we move to next choice
    //bool res = false;
    // Find the last item not = numStageElem-1
    int itemToChange  = -1;
    for(  int i= ((int)indChoice.size())-1; i>=0;    --i )
    {
        if( indChoice[i] <  numStageElem-1 )
        {
            itemToChange = i;
            break;
        }
    }
    if( itemToChange < 0 )
    {
        // No solution
        return false;
    }
    // Now we clear out everything beyond it
    for( int i= itemToChange+1; i<(int) indChoice.size(); ++i)
    {
        indChoice[i] = 0;
    }
    indChoice[itemToChange] ++;
    return true;
}

void DumpVecSequences( const vector<SEQUENCE> &vecSeqs )
{
    cout << "Vector of sequneces = \n";
    for( unsigned int i=0; i<vecSeqs.size(); ++i )
    {
        DumpSequence( vecSeqs[i] );
    }
}

void GetVecSequencesIV( const vector<SEQUENCE> &vecSeqs, int left, int right, vector<SEQUENCE> &vecSeqsIV )
{
    vecSeqsIV.clear();
    for(unsigned int i=0;i<vecSeqs.size(); ++i)
    {
        SEQUENCE ivRow;
        GetSeqInterval(vecSeqs[i], ivRow, left, right);
        vecSeqsIV.push_back( ivRow );
    }
}

int GetNumZerosInSeq(const SEQUENCE &seq)
{
    int res = 0;
    for(unsigned int i=0; i<seq.size(); ++i)
    {
        if( seq[i] == 0 )
        {
            res++;
        }
    }
    return res;
}

void GetSeqSplit(const SEQUENCE &seq, set<int> &zeroBits, set<int> &oneBits)
{
    zeroBits.clear();
    oneBits.clear();
    for(unsigned int i=0; i<seq.size(); ++i)
    {
        if( seq[i] == 0 )
        {
            zeroBits.insert( i );
        }
        else
        {
            oneBits.insert( i );
        }
    }
}

static int QSortCompareIntPair( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
    // assume sorting in accending order, and use the first value in the int pair to sort
    int n1 = ((pair<int,int> *) arg1)->first;
    int n2 = ((pair<int,int> *) arg2)->first;
//cout <<"arg1 = " << n1 << ", arg2 = " << n2 << endl;
    if( n1 > n2)
    {
        return 1;
    }
    else if( n1 < n2)
    {
        return -1;
    }
    else 
    {
        return 0;
    }
}

void SortVecIntPairs(vector<pair<int,int> > &listPairs )
{
    pair<int,int> array [ listPairs.size()  ];
    for(int i=0; i< (int)listPairs.size() ; ++i)
    {
        array[i] = listPairs[i];
    }
    qsort( (void *)array, listPairs.size(), sizeof( pair<int,int>  ), QSortCompareIntPair );
    // Now write back
    for(int i=0; i< (int)listPairs.size(); ++i)
    {
        listPairs[i] = array[i];
    }

    //delete [] array;
}


///////////////////////////////////////////////////////////////////////////////////////////
int GetSubstringLeftPos( const INTERVAL_SUBSTRING &substr )
{
    return substr.first.first;
}
int GetSubstringRightPos( const INTERVAL_SUBSTRING &substr )
{
    return substr.first.second;
}
void GetIVSubstringData(const INTERVAL_SUBSTRING &substr, SEQUENCE &seq )
{
    seq = substr.second;
}
INTERVAL GetSubstringInterval( const INTERVAL_SUBSTRING &substr)
{
    return substr.first;
}
bool GetSubstringSegment(const INTERVAL_SUBSTRING &substr, const INTERVAL &ivToRead, SEQUENCE &segment)
{
    YW_ASSERT_INFO( IsIntervalContained(ivToRead, substr.first) == true, "Two intervals do not have contained" );

    // remember we have to offset a little
    int startPos = GetSubstringLeftPos( substr );
    GetSeqInterval(substr.second, segment, ivToRead.first-startPos, ivToRead.second-startPos);
    return true;
}

int GetSubstringValAt( const INTERVAL_SUBSTRING &substr, int pos )
{
    YW_ASSERT_INFO( pos >= GetSubstringLeftPos(substr) && pos <= GetSubstringRightPos(substr), "Range error." );

    int convPos = pos - GetSubstringLeftPos( substr );
    return substr.second[ convPos ];
}

bool IsSegmentContained( const INTERVAL_SUBSTRING &seqContained, const INTERVAL_SUBSTRING& seqContainer   )
{
    // First the range has to match
    if( GetSubstringLeftPos(seqContained)  < GetSubstringLeftPos(seqContainer)  || 
        GetSubstringRightPos(seqContained)  > GetSubstringRightPos(seqContainer)  )
    {
        return false;
    }
    // Then the corresponding position must match too
    for( int p = GetSubstringLeftPos(seqContained); p<= GetSubstringRightPos(seqContained); p++ )
    {
        if( GetSubstringValAt(seqContained, p)  !=  GetSubstringValAt( seqContainer, p) )
        {
            return false;
        }
    }
    return true;
}

bool AreSegmentsConsistent( const INTERVAL_SUBSTRING &seq1, const INTERVAL_SUBSTRING& seq2 )
{
    // If disjoint, yes, it is consistent
    INTERVAL ivInt;
    bool fInt = GetIntervalOverlap( GetSubstringInterval(seq1), GetSubstringInterval(seq2), ivInt);
    if( fInt == false  )
    {
        return true;
    }
//cout << "ivInt.first = " << ivInt.first << ", ivInt.second = " << ivInt.second << endl;
    // make sure the two things matches
    SEQUENCE seqp1;
    GetSubstringSegment( seq1, ivInt, seqp1);
//cout << "seqp1 = ";
//DumpSequence( seqp1 );
    SEQUENCE seqp2;
    GetSubstringSegment( seq2, ivInt, seqp2);
//cout << "seqp2 = ";
//DumpSequence( seqp2 );

    if( seqp1 == seqp2 )
    {
        return true;
    }
    else
    {
        return false;
    }

}

int GetSegmentsIntersection( const INTERVAL_SUBSTRING &seq1, const INTERVAL_SUBSTRING& seq2, INTERVAL &iv )
{
    // we simply get how larget the intersection from the interval ONLY

    bool fInt = GetIntervalOverlap( GetSubstringInterval(seq1), GetSubstringInterval(seq2), iv);
    if( fInt == false  )
    {
        return 0;
    }
    return iv.second - iv.first+1;
}

bool AreSegmentsNextto( const INTERVAL_SUBSTRING &seq1, const INTERVAL_SUBSTRING& seq2 )
{
//cout << "seq1.left = " <<  GetSubstringLeftPos(seq1) << ", right = " <<  GetSubstringRightPos(seq1) << endl;
//cout << "seq2.left = " <<  GetSubstringLeftPos(seq2) << ", right = " <<  GetSubstringRightPos(seq2) << endl;
    // Two segments are next to each other if the can form a single bigger ungapped piece
    if( GetSubstringLeftPos(seq1) == GetSubstringRightPos(seq2) + 1
        || GetSubstringLeftPos(seq2) == GetSubstringRightPos(seq1) + 1  )
    {
//cout << "Yes, neighbours.\n";
        return true;
    }
    else
    {
//cout << "No, not neighbours.\n";
        return false;
    }
}

void DumpSubstring( const INTERVAL_SUBSTRING &substr )
{
    cout << "[" << GetSubstringLeftPos(substr) << ",";
    cout << GetSubstringRightPos(substr) << "], ";
    DumpSequence( substr.second );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// More useful functions

// This is a very useful function, so expose it
int FindMatchedSeqForFounders( const vector<SEQUENCE> &founder, const SEQUENCE &seq,
                                  set<int> &endRows, bool fPrefix)
{
    // Return the number of crossovers
    // This function computes the minimum recombination weight for the given hapRow
    // when restricted to interval [left, right] in mat
    int res = 0;

    set<int> lastTrackRows;  // set of rows that matching the hapRow

    // Ohterwise, we can start from all possible rows
    for( unsigned int i=0; i<founder.size(); ++i )
    {
        lastTrackRows.insert( i );
    }

    int curpos  = 0; 
    int end = seq.size();
    if( fPrefix == false )
    {
        curpos = seq.size()-1;
        end = -1;
    }


    while( curpos != end )
    {
        // Each time, we intersect the set with the sets matching the current bit
        set<int> trackRows;
        for(unsigned int i=0; i<founder.size(); ++i)
        {
            if( founder[i][curpos] == seq[curpos])
            {
                // Yes, this row matches
                trackRows.insert( i );
            }
        }

        // Now we test if there is intersection, if non-empty, we contiinue
        set<int> sint;
        JoinSets(trackRows, lastTrackRows, sint);
        if(sint.size() == 0)
        {
            break;
        }
        else
        {
            // In this case, we still continue
            lastTrackRows = sint;
        }

        if( fPrefix == true)
        {
            curpos++;
        }
        else
        {
            curpos--;
        }
    }

    endRows = lastTrackRows;

    // what is the length of the prefix/suffix
    if( fPrefix )
    {
        res = curpos;
    }
    else
    {
        res = seq.size() - 1 - curpos;
    }

    return res;
}
