#include <set>
#include <vector>
using namespace std;
#include "HeuristicBlock.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commonly used function here...


////////////////////////////////////////////////////////////////////////////////////////////////////////


GreedyRecBlockParser :: GreedyRecBlockParser( const BinaryMatrix &mat, int nF ) : 
    inputMat( mat ), numFounders( nF ), numOfSteps( 1000 ), maxDeviatePercent(0.3)
{

}


GreedyRecBlockParser :: ~GreedyRecBlockParser()
{
}


// Now read in from a file
int GreedyRecBlockParser :: Solve( int numTrials  )
{
    // We first build a queue for all possible intervals, weighted by their multiplicty
    // Intuitiion is to select the highest score intervals, and make them as founders
    WT_SUBSTRING_QUEUE substringQueue;

    // Initialize queue first
    BuildSubstringQueue( substringQueue );

    int minRecomb = HAP_MAX_INT;

    // Now we run for certain number of times
    for(int i=0; i<numTrials; ++i)
    {
if( ( (i+1) % 100) == 0)
{
//cout << "Trying " << i << " runs...\n";
}
        // Now we run for one time
        // run one heuristc founders
        WT_SUBSTRING_QUEUE curSSQueue = substringQueue;

        // Now we randomized a bit here
        PerturbSSQueue( curSSQueue );

        // Now see how good is this?
        FOUNDER_LIST curFounders;
        int stepScore = FindHeuristicFounders(curSSQueue, curFounders);
        if( stepScore < minRecomb )
        {
            minRecomb = stepScore;
            this->foundersBest = curFounders;
cout << "current minimum crossovers = " << minRecomb << endl;
        }

    }

    // done 
    return minRecomb;
}

int GreedyRecBlockParser :: SolveOneCall()
{
    // We first build a queue for all possible intervals, weighted by their multiplicty
    // Intuitiion is to select the highest score intervals, and make them as founders
    WT_SUBSTRING_QUEUE substringQueue;

    // Initialize queue first
    BuildSubstringQueue( substringQueue );

    FOUNDER_LIST curFounders;
    int stepScore = FindHeuristicFounders(substringQueue, curFounders);
    this->foundersBest = curFounders;

    return stepScore;
}


void GreedyRecBlockParser :: PerturbSSQueue( WT_SUBSTRING_QUEUE &curSSQueue )
{
    // We find how much we can get by some randomization
    WT_SUBSTRING_QUEUE queueNew;
    while( curSSQueue.empty() == false )
    {
        const WEIGHTED_INTERVAL_SUBSTRING &topItem = curSSQueue.top();

        // Now process it
        double fractChange = (2*GetRandFraction()-1.0)*maxDeviatePercent;
        YW_ASSERT_INFO( fractChange < 0.5, "Change is too large." );
        int scoreNew =  (int)(topItem.first *(1.0 + fractChange) );
//cout << "scoreOld = " << topItem.first << ", scorenew = " << scoreNew << endl;
        WEIGHTED_INTERVAL_SUBSTRING itemNew(scoreNew, topItem.second);
        queueNew.push( itemNew );

        // Now pop it out. WARNING: has to be done after it is processed
        curSSQueue.pop();
        
    }

    // Finally we get a new queue
    curSSQueue = queueNew;
}


int GreedyRecBlockParser :: FindHeuristicFounders (WT_SUBSTRING_QUEUE &substringQueue, FOUNDER_LIST &founders )
{
    // Initially, we have no info on founders
    // clear away what are leftover
    founderSubstrings.clear();
    for(int f = 0; f< numFounders; ++f)
    {
        vector<INTERVAL_SUBSTRING> emptySubstrings;
        founderSubstrings.push_back( emptySubstrings );
    }

    // Now we get all item in the queue and process it one by one
    while(substringQueue.empty() == false)
    {
        const WEIGHTED_INTERVAL_SUBSTRING &topItem = substringQueue.top();

        // Now process it
        ProcessFragment( topItem.second );

        // Now pop it out. WARNING: has to be done after it is processed
        substringQueue.pop();
    }

    // Now we finish the founders
    FinishFounders();

    // make sure the founders are constructed
//    DumpFounders();
    YW_ASSERT_INFO(AreFounderFullyConstructed() == true, "Founders are not completely constructed.\n");

    // what else
    int res = 0;
    //FOUNDER_LIST founders;
    GetFounder( founders );
    YW_ASSERT_INFO( CheckFounders(founders) == true, "Bad set of founders." );
    for( int r=0; r<inputMat.GetRowNum(); ++r )
    {
        SEQUENCE rowr;
        inputMat.GetRow( r, rowr );
        set<int> exitRows;
        int stepRes = CheckSeqForFounderFrom(founders, rowr, -1, exitRows );
        res += stepRes;
    }

    return res;
}


void GreedyRecBlockParser :: GetSolFounders( FOUNDER_LIST &founders )
{
    // we retrieve the best possible founders
    founders  = this->foundersBest;
}


// implementation functons
void GreedyRecBlockParser :: BuildSubstringQueue( WT_SUBSTRING_QUEUE &substringQueue)
{
    // We process the input matrix and check for each substring

    for(int ivLeft = 0; ivLeft < inputMat.GetColNum(); ++ivLeft )
    {
        for(int ivRight = ivLeft; ivRight < inputMat.GetColNum(); ++ivRight )
        {
            pair<int,int> psites( ivLeft, ivRight );

            // This is what we want
            BinaryMatrix submat;
            inputMat.SubMatrix(0, inputMat.GetRowNum()-1, ivLeft, ivRight, submat );

            // Now get multiplicity
            BinaryMatrix trimedMat = submat;
            trimedMat.TrimDupRows();

            for( int r=0; r<trimedMat.GetRowNum(); ++r)
            {
                SEQUENCE seqr;
                trimedMat.GetRow( r, seqr );

                // How many copies does this has?
                int mcRow = submat.GetMultiplictyForRow( seqr );

                // compute a score
                int score = mcRow * (ivRight-ivLeft+1);
                INTERVAL_SUBSTRING ivs( psites, seqr );
                WEIGHTED_INTERVAL_SUBSTRING wivs( score, ivs );
//DumpQueueEntry(wivs);
                // queue it
                substringQueue.push( wivs );
            }


        }
    }

//cout << "Total fragment queue size = " << substringQueue.size() << endl;
//cout << "Highest priority item = \n";
//DumpQueueEntry ( substringQueue.top() );
}

void GreedyRecBlockParser :: DumpQueueEntry( const WEIGHTED_INTERVAL_SUBSTRING & entry )
{
    // Dump out an entry for the queue 
    cout << "Interval = [";
    cout << entry.second.first.first  << ", " << entry.second.first.second <<  " ], ";
    DumpSequence( entry.second.second );
    cout << "Score = " << entry.first << endl;
}


void GreedyRecBlockParser :: ProcessFragment( const INTERVAL_SUBSTRING &fragment)
{
    // This function checks to see whether we can place this into one of the founders
    // We see this by checking:
    // a.0: if this fragment is contained in a founder, do nothing
    // a. We must reject it if adding this would make one column completely uniform. If we add it, we will 
    // not get valid founder set
    // b. If no above condition occurs, we then try to merge this with existing ones. If we can do this
    // this is better since we are fixing less founders
    // c. If the above is not possible, simply choose the first avaible slot to put it in
//cout << "Processing fragment ";
//DumpSubstring(fragment);
    if( IsFragmentResolved( fragment ) == true )
    {
//cout << "This piece has already been resolved.\n";
        return;
    }

    // This queue contains to what extent a founder overlaps this fragment 
    priority_queue< pair<int, int> > queueFounderOverlapped;
    for(int f = 0; f< numFounders; ++f )
    {
        //vector< INTERVAL > addedIVs;
        int totAddedBits = 0;
        bool fCompat = CheckFragmentForFounder( f, fragment, totAddedBits );

        if( fCompat == false )
        {
//cout << "Founder " << f << " is not compatible.\n";
            continue;
        }
//cout << "This founder has overlapped score = " << totAddedBits << endl; 

        // we do want to highest number
        pair<int,int> pp(  totAddedBits, f );
        queueFounderOverlapped.push( pp );
    }

    // Now if no founder is possible, reject this string
    if( queueFounderOverlapped.empty() == true )
    {
//cout << "This string is not compatible with existing ones.\n";
        return;
    }

    // Now we add this substring to the top founder
    int topFounder = queueFounderOverlapped.top().second;
    AddFragmentToFounder( fragment, topFounder  );
//DumpFounders();
}


bool GreedyRecBlockParser :: IsFragmentResolved( const INTERVAL_SUBSTRING &fragment ) 
{
    // This fragment is resolved if it contains inside a founder segment
    for( int f = 0; f< numFounders; ++f )
    {
        if( IsFragmentResolvedIn(fragment, f )  == true )
        {
            return true;
        }
    }
    return false;
}

bool GreedyRecBlockParser :: IsFragmentResolvedIn( const INTERVAL_SUBSTRING &fragment, int fIndex )  
{
    // Check this founder to see if contains segments that works here
    for( unsigned int p = 0; p<founderSubstrings[fIndex].size(); ++p )
    {
        if( IsSegmentContained( fragment, founderSubstrings[fIndex][p] ) == true  )
        {
            return true;
        }
    }
    return false;
}

bool  GreedyRecBlockParser :: CheckFragmentForFounder( int fIndex, const INTERVAL_SUBSTRING &fragment, 
                              int &totAddedBits )
{
    // We first check to see if it is consistent with it or not. If not, reject

    bool fConsist = AreFragmentFounderConsist( fragment, fIndex );
    if(fConsist == false)
    {
//cout << "Not consistent.\n";
        return false;
    }


    // Now we figoure the cost of adding this fragment
    set<int> dupSites;
    int bits = 0;
    for( unsigned int p = 0; p<founderSubstrings[fIndex].size(); ++p )
    {
        // Check for each founder fragment
        INTERVAL ivInt;
        int curBits = GetSegmentsIntersection( fragment, founderSubstrings[fIndex][p], ivInt );

        if( curBits == 0 )
        {
            continue;
        }
        bits += curBits;

        for( int ppp = ivInt.first; ppp <=ivInt.second; ++ppp )
        {
//cout << ppp << " is dup site.\n";
            dupSites.insert( ppp );
        }
    }

    // Now we need to make sure we are not making this becomes invalid
    set< pair<int, int> > dangerSpots;
    GetDangerSpots( dangerSpots  );
    for(int p = GetSubstringLeftPos(fragment); p <= GetSubstringRightPos(fragment); ++p)
    {
        // if this pos is already in the founder, skip
        if( dupSites.find( p ) != dupSites.end() )
        {
            continue;
        }

        //YW_ASSERT_INFO( fragment.first.second - fragment.first.first < (int)fragment.second.size(), "Fragment info wrong." );
        //int val = fragment.second[ p- fragment.first.first];
        int val = GetSubstringValAt( fragment, p );
        pair<int,int> pSiteVal( p, val );
        if( dangerSpots.find( pSiteVal ) != dangerSpots.end() )
        {
            // can not add this bit
//cout <<"Can not add p = " << p << " with val = " << val << endl;
            return false;
        }
    }

#define DETERMINISTIC_PLACEMENT_ONLY   1

    // Good we are fine
#ifndef DETERMINISTIC_PLACEMENT_ONLY
    int rndDev = (int)( GetRandFraction() * inputMat.GetColNum() );
    totAddedBits = bits * inputMat.GetColNum() + rndDev;
#else
    totAddedBits = bits;
#endif

    return true;

}

void GreedyRecBlockParser :: GetDangerSpots( set< pair<int, int> > & dangerSpots  )
{
    dangerSpots.clear();

    // This function finds the sites that are dangeriously low on bits
    // Danger bits are defined to be positions that have uniformly being selected as (nFounder-1) 0 (or all 1)
    for( int p = 0; p< inputMat.GetColNum(); ++p)
    {
        int nFounder0 = 0, nFounder1 = 0;

        for( int f = 0; f<numFounders; ++f )
        {
            // Check to see this founder has 0 or 1
            int fBitVal = GetFounderBit( f, p  );
            if( fBitVal < 0 )
            {
                continue;
            }
            if( fBitVal == 0 )
            {
                nFounder0 ++;
            }
            else if( fBitVal == 1 )
            {
                nFounder1 ++;
            }
            else
            {
                YW_ASSERT_INFO(false, "Founder bit wrong.");
            }
        }

        // If this position in danger
        if( nFounder0 == 0 && nFounder1 == numFounders-1 )
        {
            pair<int,int> pp( p, 1);
            dangerSpots.insert( pp );
//cout << p << " is a danger site.\n";
        }
        else if( nFounder1 == 0 && nFounder0 == numFounders-1 )
        {
            pair<int,int> pp( p, 0);
            dangerSpots.insert( pp );
//cout << p << " is a danger site.\n";
        }
    }
}

int GreedyRecBlockParser :: GetFounderBit( int fIndex, int pos  )
{
    // Return -1 if the bit is missing
    int res = -1;

    for(unsigned int i = 0; i<founderSubstrings[fIndex].size(); ++i)
    {
        if( GetSubstringLeftPos ( founderSubstrings[fIndex][i] ) <= pos && 
            GetSubstringRightPos ( founderSubstrings[fIndex][i] ) >= pos )
        {
            res = GetSubstringValAt(founderSubstrings[fIndex][i], pos);
            break;
        }
    }

    return res;
}

bool GreedyRecBlockParser :: AreFragmentFounderConsist( const INTERVAL_SUBSTRING &fragment, int fIndex )
{
    // Is this fragment good for founder? Each founder segment must be consistent with this fragment
    for( unsigned int p = 0; p<founderSubstrings[fIndex].size(); ++p )
    {
        // Check for each founder fragment
        if( AreSegmentsConsistent( fragment, founderSubstrings[fIndex][p] ) == false )
        {
            return false;
        }
    }
    return true;
}


void GreedyRecBlockParser :: AddFragmentToFounder( const INTERVAL_SUBSTRING &fragment, int fIndex  )
{
//cout << "On entry AddFragmentToFounder: adding segment to founder " <<  fIndex << endl;
//DumpSubstring(fragment);
//DumpFounders();
    // We essentially fill in the values of gaps and make one larger blocks
    vector<INTERVAL_SUBSTRING> listNewFounderStrings;
    vector<INTERVAL_SUBSTRING> listNewFStringsPre;
    vector<INTERVAL_SUBSTRING> listNewFStringsPost;
    vector<INTERVAL_SUBSTRING> listNewFStringsMid;
    
    for( unsigned int p = 0; p<founderSubstrings[fIndex].size(); ++p )
    {
        INTERVAL iv;
        int nOverlap = GetSegmentsIntersection( fragment, founderSubstrings[fIndex][p], iv );
        if( nOverlap == 0  && AreSegmentsNextto(fragment, founderSubstrings[fIndex][p]) == false )
        {
            // put it back in. But first test whether this is before and after the fragment
            if( GetSubstringRightPos(founderSubstrings[fIndex][p]) < GetSubstringLeftPos(fragment)  )
            {
//cout << "adding this piece of founder to pre queue: ";
//DumpSubstring(founderSubstrings[fIndex][p]);
                listNewFStringsPre.push_back( founderSubstrings[fIndex][p] );;
            }
            else
            {
//cout << "adding this piece of founder to post queue: ";
//DumpSubstring(founderSubstrings[fIndex][p]);
                listNewFStringsPost.push_back( founderSubstrings[fIndex][p] );;
            }
            continue;
        }

        // Anything intersect must be combined together
        listNewFStringsMid.push_back( founderSubstrings[fIndex][p] );
//cout << "adding this piece of founder to mid queue: ";
//DumpSubstring(founderSubstrings[fIndex][p]);

    }
    // Now we first build up the new piece of founders
    INTERVAL_SUBSTRING stringNew;
    // check to see which one to start
    bool fStartExist = false, fEndExist = false;
    int startPos = GetSubstringLeftPos(fragment), endPos = GetSubstringRightPos( fragment );
    if( listNewFStringsMid.size() > 0 && 
        GetSubstringLeftPos( listNewFStringsMid[0] ) < GetSubstringLeftPos( fragment )  )
    {
        fStartExist = true;
        startPos = GetSubstringLeftPos( listNewFStringsMid[0] );
    }
    if( listNewFStringsMid.size() > 0 && 
        GetSubstringRightPos( listNewFStringsMid[ listNewFStringsMid.size()-1 ] ) > GetSubstringRightPos( fragment )  )
    {
        fEndExist = true;
        endPos = GetSubstringRightPos( listNewFStringsMid[ listNewFStringsMid.size()-1 ] ) ;
    }    
//    YW_ASSERT_INFO( fStartExist == false || fEndExist == false, "Can not be both true here" );
    SEQUENCE seqNew;
    for( int ppp = startPos; ppp <= endPos; ++ppp )
    {
        int valPos;
        if( ppp < GetSubstringLeftPos( fragment )  )
        {
            valPos = GetSubstringValAt( listNewFStringsMid[0] , ppp );
        }
        else if( ppp > GetSubstringRightPos( fragment ) )
        {
            valPos = GetSubstringValAt( listNewFStringsMid[ listNewFStringsMid.size()-1 ] , ppp );
        }
        else
        {
            // then take it from fragment
            valPos = GetSubstringValAt( fragment, ppp );
        }
        seqNew.push_back( valPos );
    }
    stringNew.first.first = startPos;
    stringNew.first.second = endPos;
    stringNew.second = seqNew;
//cout << "New string = ";
//DumpSubstring( stringNew );

    // Finally we concat the founders
    founderSubstrings[fIndex].clear();
    for(unsigned int i=0; i<listNewFStringsPre.size(); ++i)
    {
        founderSubstrings[fIndex].push_back( listNewFStringsPre[i]  );
    }
    founderSubstrings[fIndex].push_back( stringNew );
    for(unsigned int i=0; i<listNewFStringsPost.size(); ++i)
    {
        founderSubstrings[fIndex].push_back( listNewFStringsPost[i]  );
    }
    // Verify the founders here
//cout << "verify founders...\n";
    VerifyPartialFounders();
}

bool GreedyRecBlockParser :: AreFounderFullyConstructed()
{
    // Are all founder fully constructed
    for( int f=0; f<numFounders; ++f )
    {
        if( founderSubstrings[f].size() != 1 || GetSubstringLeftPos( founderSubstrings[f][0] ) != 0 || 
            GetSubstringRightPos(  founderSubstrings[f][0]  ) != inputMat.GetColNum()-1)
        {
//DumpFounders();
            return false;
        }
    }
    return true;
}

void GreedyRecBlockParser :: GetFounder( FOUNDER_LIST &founders )
{
    founders.clear();

    // retrieve the set of constructed founders
    for( int f=0; f<numFounders; ++f )
    {
        const SEQUENCE &seq = founderSubstrings[f][0].second;
        YW_ASSERT_INFO( (int)seq.size() == inputMat.GetColNum(), "Founder not complete"  );
        founders.push_back( seq );;
    }

}

void GreedyRecBlockParser :: DumpFounders()
{
    cout << "Raw founders are: \n";
    for( int f=0; f<numFounders; ++f )
    {
        for( unsigned int pos = 0; pos < founderSubstrings[f].size(); ++pos  )
        {
            cout << "Left = " << GetSubstringLeftPos( founderSubstrings[f][pos] ) << endl ;
            cout << "Right = " << GetSubstringRightPos( founderSubstrings[f][pos] ) << endl ;
            DumpSequence( founderSubstrings[f][pos].second );
        }
    }



    cout << "Founders are: \n";
    for( int f=0; f<numFounders; ++f )
    {
        unsigned int pos = 0;
        int lastBitPos = -1;
        while( pos < founderSubstrings[f].size()  )
        {
            // First dump out bits from last bit up to the current bit
            for( int ppp = lastBitPos+1; ppp < GetSubstringLeftPos( founderSubstrings[f][pos] ); ++ppp  )
            {
                cout << "*";
            }
            // Then dump out the entire segments
            for(int pp1 = GetSubstringLeftPos( founderSubstrings[f][pos] ); 
                pp1 <= GetSubstringRightPos( founderSubstrings[f][pos] ); ++pp1)
            {
                cout << GetSubstringValAt( founderSubstrings[f][pos] , pp1 );
            }

            // Now move to the next bit
            lastBitPos = GetSubstringRightPos( founderSubstrings[f][pos] );
            pos++;
        }

        // Fill up the last bit here
        for(int pp2 = lastBitPos+1; pp2 <= inputMat.GetColNum()-1; ++pp2)
        {
            cout << "*";
        }

        // finish the line
        cout << endl;
    }

}

void GreedyRecBlockParser ::  VerifyPartialFounders()
{
    // This function verify to ensure the founders are build correctly
    // Right now, it check to see if there are contiguous blocks of founders that should be linked
    for(int f=0; f<numFounders; ++f)
    {
        for( int b = 0; b < (int)founderSubstrings[f].size()-1; ++b)
        {
            if( GetSubstringRightPos( founderSubstrings[f][b] ) +1 == GetSubstringLeftPos(founderSubstrings[f][b+1] ) )
            {
                DumpFounders();
                YW_ASSERT_INFO( false, "Can not have founders that are neighbours but un-merged." );
            }
        }
    }
}

void GreedyRecBlockParser :: FinishFounders()
{
    // For some reason, the founders may not be all done. In that case, for now
    // we simply fill in arbitary value to gaps (here the majority state if there is already a state in that site)
    for( int f=0; f<numFounders; ++f )
    {
        if( founderSubstrings[f].size() != 1 || GetSubstringLeftPos( founderSubstrings[f][0] ) != 0 || 
            GetSubstringRightPos(  founderSubstrings[f][0]  ) != inputMat.GetColNum()-1)
        {
//cout << "Here, we are not done yet.\n";
//DumpFounders();
            // This is not a good founder
            INTERVAL_SUBSTRING founder;
            founder.first.first = 0;
            founder.first.second = inputMat.GetColNum()-1;

            // Now we start to fill in byte by byte
            for(int p=0; p<inputMat.GetColNum(); ++p)
            {
                founder.second.push_back( 0 );
            }

            // Now read in the data
            for( unsigned int ivs = 0; ivs<founderSubstrings[f].size(); ++ivs )
            {
                int leftPos = GetSubstringLeftPos( founderSubstrings[f][ivs]) ;
                SEQUENCE data;
                GetIVSubstringData( founderSubstrings[f][ivs], data );
                for( unsigned int ii = 0; ii<data.size(); ++ii )
                {
                    founder.second[ leftPos + ii ] = data[ii] ;
                }
            }

            // Now fill in the left overs with 0, for now, since that will be if not updated
            // TBD

            // Now set it here
            founderSubstrings[f].clear();
            founderSubstrings[f].push_back( founder );

        }

    }

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Useful functions
// Some useful functions
const int MAX_BRUTE_FORCE_SITE = 10;

bool CheckFounders( FOUNDER_LIST &founders )
{
    // Right now, this function only checks whether each site is not all uniform
    if( founders.size() == 0 )
    {
        return true;
    }

    int nSites = founders[0].size();

    for( int i=0; i< nSites; ++i)
    {
        bool fZero = false, fOne = false;
        for( unsigned int f = 0; f< founders.size(); ++f)
        {
            if( founders[f][i] == 0 )
            {
                fZero = true;
            }
            else 
            {
                fOne = true;
            }
            if( fZero == true && fOne == true )
            {
                break;
            }
        }
        if( fZero == false || fOne == false )
        {
            return false;
        }
    }
    return true;    // good founders
}


int CalcMinCrossoverBruteForce( const BinaryMatrix &inputMat, int nFounders )
{
    // This function simply enuerates all possible founders and compute a number
    YW_ASSERT_INFO( inputMat.GetColNum() <= MAX_BRUTE_FORCE_SITE, "Too many sites to handle." );

    // now we have all possible sets
    int dataWidth = inputMat.GetColNum();
    int numHCSeqs = (0x1 << inputMat.GetColNum() );
    set<int> allPossibleSeqs;
    for(int i=0; i< (0x1 << inputMat.GetColNum() ); ++i)
    {
        allPossibleSeqs.insert( i );
    }


    // Now we enumerate all possible things
    vector<int> posvec;
    GetFirstCombo( nFounders, numHCSeqs,  posvec );
    int minRecomb = HAP_MAX_INT;
    while(true)
    {
        // now we get the corresponding set of founders
        FOUNDER_LIST founders;

        for( unsigned int i = 0; i<posvec.size(); ++i )
        {
            SEQUENCE seq;
            GetHyperCubeSeq( posvec[i], seq, dataWidth);
            founders.push_back( seq );
        }

        // is this set of founders good?
        if( CheckFounders( founders) == true )
        {
            // Now we compute min-recombs for these founders
            int totNum = 0;
            for(int r = 0; r<inputMat.GetRowNum(); ++r)
            {
                SEQUENCE rowr;
                inputMat.GetRow( r, rowr );
                set<int> exitRows;
                totNum += CheckSeqForFounderFrom( founders, rowr, -1, exitRows );
            }

            // Is this one good?
            if( totNum < minRecomb )
            {
                minRecomb = totNum;
            }

        }

        // now we get next combo
        if( GetNextCombo( nFounders, numHCSeqs, posvec) == false )
        {
            break;
        }
    }
    return minRecomb;
}

