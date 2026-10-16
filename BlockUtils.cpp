#include <set>
#include <vector>
#include <map>
#include <stack>
#include <queue>
using namespace std;
#include "BlockUtils.h"
#include "Utils2.h"
#include "Utils3.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool AreSetContained( const set< set<int> > &setsCheck )
{
    vector< set<int> > listSets;
    for( set< set<int> > :: iterator it = setsCheck.begin(); it != setsCheck.end(); ++it )
    {
        listSets.push_back( *it );
    }

    // now check whether one set is contained in another
    for( unsigned int i=0; i<listSets.size(); ++i )
    {
        for(unsigned int j=i+1; j<listSets.size(); ++j)
        {
            if( IsSetContainer(listSets[i], listSets[j] ) == false 
                && IsSetContainer( listSets[j], listSets[i] ) == false )
            {
                return false;
            }
        }
    }
    return true;
}


// Is this founder a good one?
static bool IsFounderValid( const FOUNDER_LIST &founder, int wndSize, int nFounders)
{
//    cout << "founder.size() = " << founder.size() << endl;
    //YW_ASSERT_INFO( founder.size() == nFounders, "Founder size wrong." );
//cout << "IsFounderValid :: Founder = \n";
//for(unsigned int k=0; k<founder.size(); ++k)
//{
//    DumpSequence( founder[k]);
//}

    // We first amek sure the founder has at least 1/0 in any site
    for(  int i=0; i<wndSize; ++i )
    {
        bool seeZero = false, seeOne = false;
        for(  int j=0; j<nFounders; ++j)
        {
            if( founder[j][i] == 0 )
            {
                seeZero = true;
            }
            else
            {
                seeOne = true;
            }
        }
        if( seeZero == false || seeOne == false )
        {
            // has a uniform one, do not want to continue
//cout << "bad due to uniform\n";
            return false;
        }
    }

#if 0
    // Now we also make sure two neighour rows must not be same or complemetry
    for(  int i=0; i<wndSize-1; ++i )
    {
        int samePair = 0;
        for(  int j=0; j<nFounders; ++j)
        {
            if( founder[j][i] == founder[j][i+1] )
            {
                samePair ++;
            }
            
        }
        if(  samePair ==  0 || samePair == nFounders  )
        {
            // has a uniform one, do not want to continue
//cout << "neighour same. bad\n";
            return false;
        }
    }
#endif

    // now it is good
    return true;
}

static bool IsFounderWithIdentRows( const FOUNDER_LIST &founder, int wndSize, int nFounders)
{
    // at last, we do not allow identical rows
    for(unsigned int r1= 0; r1 < founder.size(); ++r1 )
    {
        for(unsigned r2 =r1+1; r2 < founder.size(); ++r2)
        {
            if( founder[r1] == founder[r2] )
            {
                return true;
            }
        }
    }


    return false;
}

static bool IsSeqMatchFounder (const SEQUENCE &seq, const FOUNDER_LIST &founder ) 
{
    // at last, we do not allow identical rows
    for(unsigned int r1= 0; r1 < founder.size(); ++r1 )
    {
        if( founder[r1] == seq )
        {
            return true;
        }
    }


    return false;
}


static void ConsFounderRec( int wndPos, int wndSize, int nFounders, vector<FOUNDER_LIST>&founderCandidates)
{
    if( wndPos == wndSize )
    {
        // stop
        return;
    }

    vector<FOUNDER_LIST> foundersNew;

    if( founderCandidates.empty() == true  )
    {
        for(int n = 0; n < nFounders-1; ++n)
        {
            SEQUENCE seq;
            GetHyperCubeSeq( (0x1 << (n+1) ) -1, seq, nFounders);
            FOUNDER_LIST simpleFd;
            for(  int i=0; i<nFounders; ++i )
            {
                vector<int> seq0;
                seq0.push_back( seq[i] );
                simpleFd.push_back( seq0 );
            }
            foundersNew.push_back( simpleFd );
        }
    }
    else
    {

        // Otherwise, we append a new site to the existing founders
       // In this case we should append a new set of empty sequences first
        for(int n = 0; n < (0x1 << nFounders); ++n)
        {
            SEQUENCE seq;
            GetHyperCubeSeq(n, seq, nFounders);

            // Check if stack is empty, if so, populate it with a single bit founder
            //if( founderCandidates.empty() == true  )
            //{
            //    FOUNDER_LIST simpleFd;
            //    for(  int i=0; i<nFounders; ++i )
            //    {
            //        vector<int> seq0;
            //        seq0.push_back( seq[i] );
            //        simpleFd.push_back( seq0 );
            //    }
            //    foundersNew.push_back( simpleFd );
            //}
            //else
            {
                // We simply append the bit to every one current known

                for( unsigned ii = 0; ii < founderCandidates.size(); ++ii )
                {
                    FOUNDER_LIST curList = founderCandidates[ii];
                    for(unsigned int jj=0; jj<seq.size(); ++jj)
                    {
                        curList[jj].push_back( seq[jj] );
                    }

                    // Make sure this is good founder
                    if( IsFounderValid( curList,  wndPos+1, nFounders ) == true )
                    {
                        foundersNew.push_back( curList );;
                    }
                }

            }

        }
    }
    // Now update founder set
    founderCandidates = foundersNew;
cout << "foundersNew.size() = " << foundersNew.size() << endl;

    // recursive to call next
    ConsFounderRec( wndPos+1, wndSize, nFounders, founderCandidates );
}

// This functions figure out the needed mutation sites for founders and a sequence
static void FindMutSitesForFounders( const FOUNDER_LIST &founders, const SEQUENCE &seq, vector<int> &mutSites )
{
    mutSites.clear();
    for( unsigned int c = 0; c<founders[0].size(); ++c )
    {
        bool fUniform = true;
        bool fZero = false, fOne = false;
        for(unsigned int r = 0; r<founders.size(); ++r)
        {
            if( founders[r][c] == 0 )
            {
                fZero = true;
            }
            else
            {
                fOne = true;
            }
            if( fZero && fOne )
            {
                fUniform = false;
                break;
            }
        }
        if( fUniform == false )
        {
            continue;
        }

        // Do we need to mutate it?
        if( founders[0][c] !=  seq[c] )
        {
            // yes, need mutate it
            mutSites.push_back( c );
        }
    }

}

static bool CheckSeqForFounder( FOUNDER_LIST &founder,  const SEQUENCE &seq ) 
{
    // The founder is good for sequence if the optimal value (and exit row) is independent of the entry point

    vector<int> rowsStartAtCost;
    vector< set<int> > rowsStartEndRows;

    int numFirstSiteMatch = 0;

    // Now start form all possible rows
    for(unsigned int i=0; i<founder.size(); ++i)
    {
        if( founder[i][0] == seq[0] )
        {
            // Yes, this is a good start
            set<int> endRows;
            int cost = CheckSeqForFounderFrom( founder, seq, i, endRows);
            rowsStartAtCost.push_back( cost );
            rowsStartEndRows.push_back( endRows );
            numFirstSiteMatch++;
        }
    }

    // Store the end partition for seq[lastbit]
    set<int> rowsLastSiteForSeq;
    for( unsigned int r=0; r<founder.size(); ++r )
    {
        if( founder[r][ seq.size()-1 ] == seq[seq.size()-1] )
        {
            rowsLastSiteForSeq.insert( r );
        }
    }

    // Now check to see if the results are homogeneious. That is, either there is a single row achieves
    // maximum. Or all rows has same maximum (and end rows are the same)

    const int minScore = 10000;

    int numMax = 0;
    int valMax = minScore;
    set<int> endRowsMax;
    for(unsigned int i=0; i<rowsStartAtCost.size(); ++i)
    {
        if( rowsStartAtCost[i] < valMax )
        {
            numMax = 1;
            valMax = rowsStartAtCost[i];
            endRowsMax = rowsStartEndRows[i];
        }
        else if( rowsStartAtCost[i] == valMax)
        {
            numMax ++;
        }
    }

    if( numMax == 1 &&  (endRowsMax.size() == 1 || endRowsMax == rowsLastSiteForSeq) )
    {
        return true;
    }
    else if(numMax == numFirstSiteMatch)
    {
        // Make sure end rows match too
        for(unsigned int ii=0; ii< rowsStartEndRows.size(); ++ii)
        {
            if(rowsStartEndRows[ii] != endRowsMax)
            {
                // fail
cout << "Bad, because not symetric.\n";
                return false;
            }
        }
        return true;
    }
    else
    {
cout << "Bad because there are between 1 to max number of starting rows having max score.\n";
        return false;
    }

}


static bool CheckSeqForFounderExitStatus( FOUNDER_LIST &founder,  const SEQUENCE &seq ) 
{
//cout << "CheckSeqForFounderExitStatus : \n";
    // The founder is good for sequence if the optimal value (and exit row) is independent of the entry point
//cout << "This founder is bad for seq = ";
//DumpSequence( seq);
//cout << "Founder = \n";
//for(unsigned int k=0; k<founder.size(); ++k)
//{
//    DumpSequence( founder[k]);
//}


    vector<int> rowsStartAtCost;
    vector< set<int> > rowsStartEndRows;

    int numFirstSiteMatch = 0;

    // Now start form all possible rows
    for(unsigned int i=0; i<founder.size(); ++i)
    {
        if( founder[i][0] == seq[0] )
        {
            // Yes, this is a good start
            set<int> endRows;
            int cost = CheckSeqForFounderFrom( founder, seq, i, endRows);
            rowsStartAtCost.push_back( cost );
            rowsStartEndRows.push_back( endRows );
            numFirstSiteMatch++;
//cout << "For i = " << i << ", cost = " << cost << ", endRows = ";
//DumpIntSet( endRows );
        }
    }

    // Store the end partition for seq[lastbit]
    vector<int> rowsEntrySiteForSeq;
    for( unsigned int r=0; r<founder.size(); ++r )
    {
        if( founder[ r ][ 0 ] == seq[ 0 ] )
        {
            rowsEntrySiteForSeq.push_back( r );
        }
    }
//cout << "rowsEntrySiteForSeq = ";
//DumpIntVec( rowsEntrySiteForSeq );

    // Now check to see if the results are homogeneious. That is,
    // for any subset of entry status, how many exit status there are
    set< set<int> > minExitRowsSet;

    for( unsigned  int nr = 1; nr <= rowsEntrySiteForSeq.size(); ++nr )
    {
        // Try take all subset of this size
        vector<int> vecpos;
        GetFirstCombo( nr, rowsEntrySiteForSeq.size(), vecpos );

        while(true )
        {
            const int minScore = 10000;

            // first find what is the mimui score
            int valMin = minScore;
            for(unsigned int i=0; i<vecpos.size(); ++i)
            {
                YW_ASSERT_INFO( vecpos[i] < (int) rowsStartAtCost.size(), "Somethign wrong" );
                if( rowsStartAtCost[ vecpos[i] ] < valMin )
                {
                    valMin = rowsStartAtCost[ vecpos[i] ];
                }
            }
            set<int> exitRows;
            for(unsigned int i=0; i<vecpos.size(); ++i)
            {
                if( rowsStartAtCost[ vecpos[i] ] == valMin )
                {
                    UnionSets( exitRows, rowsStartEndRows[ vecpos[i] ]  );
                }
            }
            YW_ASSERT_INFO( exitRows.size() > 0, "Must not be empty here" );

            // append it here
            minExitRowsSet.insert( exitRows );

//cout << "For ";
//DumpIntVec( vecpos );
//cout << "minExitRows = ";
//DumpIntSet( exitRows );
            // enumerate next
            if( GetNextCombo(nr, rowsEntrySiteForSeq.size(), vecpos) == false )
            {
                break;
            }
        }
    }
//exit(1);
    if( AreSetContained(minExitRowsSet)  == false )
    {
cout << "This founder is bad for seq = ";
for( set< set<int> > :: iterator it = minExitRowsSet.begin(); it != minExitRowsSet.end(); ++it )
{
    DumpIntSet( *it );
}
DumpSequence( seq);
cout << "Founder = \n";
for(unsigned int k=0; k<founder.size(); ++k)
{
    DumpSequence( founder[k]);
}
        cout << "Exit sets are not contained.\n";
//        exit(1);
    }

    // if the exit rows are not unique, dump out 
    if( minExitRowsSet.size() > 1 )
    {
        cout << "For this sequence, min exit rows are not unique.\n";

        for( set< set<int> > :: iterator it = minExitRowsSet.begin(); it != minExitRowsSet.end(); ++it )
        {
            DumpIntSet( *it );
        }
cout << "This founder is bad for seq = ";
DumpSequence( seq);
cout << "Founder = \n";
for(unsigned int k=0; k<founder.size(); ++k)
{
    DumpSequence( founder[k]);
}
    }

    if( minExitRowsSet.size() > 1 )
    {
        return false;
    }

    return true;
}


// calculate the matrix for the given founder number
static int CalcLocalGenoBound( const GenotypeMatrix &submat, int nFounders )
{
    //vector< int > rowMultiplicity;
//cout << "Genotype submat = ";
//submat.Dump();
    // retrieve the set of genotyeps
    vector<SEQUENCE> listGenotypes;
    for( int r =0; r<submat.GetRowNum(); ++r )
    {
        SEQUENCE row;
        submat.GetRow(r, row);
        listGenotypes.push_back(row);
    }

    // Now get the set of all possible sequences
    int dataWidth = submat.GetColNum();
    priority_queue<int> queueFounders;
    for(int hc = 0; hc < (0x1 << dataWidth); ++hc)
    {
        // now get the sequence
        SEQUENCE seqhc;
        GetHyperCubeSeq(hc, seqhc, dataWidth);
//cout << "seqhc = ";
//DumpSequence( seqhc );
        // Now find out how often this agree with it
        int numOccurs = 0;

        for(unsigned int r = 0; r < listGenotypes.size(); ++r)
        {
            // can this seq phase this row
            if( AreHapGenoRowCompatible( seqhc, listGenotypes[r] ) == true )
            {
                if( IsSequenceHaplotype( listGenotypes[r] ) == true )
                {
                    numOccurs += 2;
                }
                else
                {
                    numOccurs += 1;
                }
            }
        }
//cout << "This sequence occurs " << numOccurs << endl;
        // add this to queue
        queueFounders.push( numOccurs );
    }

    // Now we take the top nFounder of the list
    int totSeqsExplained = 0;
    int numPoped = 0;
    while(queueFounders.empty() == false  && numPoped < nFounders)
    {
        totSeqsExplained += queueFounders.top();
        queueFounders.pop();;
        numPoped ++;
    }

    int res = 2* submat.GetRowNum() - totSeqsExplained;
    if( res < 0 )
    {
        res = 0;
    }
    return res;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interfact functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// This file contains some commonly used tools for block recobmination

int ComputeBlockLB( const BinaryMatrix &matInput, int nFounders, map<INTERVAL, int> *pBdIntervals)
{
    int res = 0;

    // the idea is to compute for each interval, the minimum number of 
    // crossover esimated
    map<INTERVAL, int> mapIntervalBds;

    for(int ivLeft = 0; ivLeft < matInput.GetColNum(); ++ivLeft )
    {
        for(int ivRight = ivLeft + 1; ivRight < matInput.GetColNum(); ++ivRight)
        {
//cout << "ivLeft = " << ivLeft << ", ivRight = " << ivRight << endl;
            // now we want to find out how much of it
            BinaryMatrix submat;
            matInput.SubMatrix( 0, matInput.GetRowNum()-1, ivLeft, ivRight, submat );

            // get multilicity of rows
            map<SEQUENCE, int> mapRowMultiplicity;
            for(int r = 0; r< submat.GetRowNum(); ++r)
            {
                SEQUENCE row;
                submat.GetRow( r, row );
                if( mapRowMultiplicity.find( row ) == mapRowMultiplicity.end() )
                {
                    mapRowMultiplicity.insert( map<SEQUENCE,int> :: value_type (row, 1) );
                }
                else
                {
                    mapRowMultiplicity[row]++;
                }
            }

            vector<int> vecMultiplicty;
            for( map<SEQUENCE, int> :: iterator it = mapRowMultiplicity.begin(); it != mapRowMultiplicity.end(); ++it )
            {
                vecMultiplicty.push_back(  it->second );
            }

            // sort it
            SortIntVec( vecMultiplicty );
//cout << "After sorting, multiplicty = ";
//DumpIntVec( vecMultiplicty );

            // Now figure out which one to take
            int localBd = 0;
            int sizeVecMultiplicty = (int) ( vecMultiplicty.size() );
            for(int ii = 0; ii < sizeVecMultiplicty - nFounders; ++ii)
            {
                localBd += vecMultiplicty[ii];
            }
//cout << "Local bd = " << localBd << endl;
            // Now add this entry to local bound
            INTERVAL iv( ivLeft, ivRight);
            mapIntervalBds.insert( map<INTERVAL, int> :: value_type( iv, localBd ) );
        }
    }

    // Nnow this is the global bound
    vector<int> localBreakpoints;
    res = CalcCompositeBound( mapIntervalBds, 0, matInput.GetColNum()-1, localBreakpoints);

    // do we need to return local bounds?
    if( pBdIntervals != NULL )
    {
        *pBdIntervals = mapIntervalBds;
    }

    return res;
}


// we do not go beyond this range for now
const int MAX_SUBMAT_SITE_NUM = 10;

int ComputeBlockGenoLB( const GenotypeMatrix &matInput, int nFounders, map<INTERVAL, int> *pBdIntervals )
{
    // This function computes a genotype lower bound
    // The basic idea is to enumerate for all intervals, the smallest number of unexplained 
    // haplotypes. To do this, we approximate by enumerating all intervals
    int res = 0;

    // the idea is to compute for each interval, the minimum number of 
    // crossover esimated
    map<INTERVAL, int> mapIntervalBds;

    for(int ivLeft = 0; ivLeft < matInput.GetColNum(); ++ivLeft )
    {
        for(int ivRight = ivLeft + 1; ivRight < matInput.GetColNum() && 
            ivRight < ivLeft + MAX_SUBMAT_SITE_NUM; ++ivRight)
        {
//cout << "ivLeft = " << ivLeft << ", ivRight = " << ivRight << endl;
            // now we want to find out how much of it
            GenotypeMatrix submat;
            matInput.SubMatrix( 0, matInput.GetRowNum()-1, ivLeft, ivRight, submat );

            int localBd = CalcLocalGenoBound( submat, nFounders );


//cout << "Local bd = " << localBd << endl;
            // Now add this entry to local bound
            INTERVAL iv( ivLeft, ivRight);
            mapIntervalBds.insert( map<INTERVAL, int> :: value_type( iv, localBd ) );
        }
    }

    // Nnow this is the global bound
    vector<int> localBreakpoints;
    res = CalcCompositeBound( mapIntervalBds, 0, matInput.GetColNum()-1, localBreakpoints);

    // do we need to return local bounds?
    if( pBdIntervals != NULL )
    {
        // we strengthen the local bounds by composition
        // For now, we only care about intervals ending in distnce
        for(int ivLeft = 0; ivLeft < matInput.GetColNum()-1; ++ivLeft )
        {
            vector<int> localBreakpoints1;
            int tightenLB = CalcCompositeBound( mapIntervalBds, ivLeft, matInput.GetColNum()-1, localBreakpoints1);
//cout << "tightenLB = " << tightenLB << ", for ivLeft = " << ivLeft << endl;
            INTERVAL iv( ivLeft, matInput.GetColNum()-1 );
            mapIntervalBds[iv] = tightenLB;
        }

        // 
        *pBdIntervals = mapIntervalBds;
    }

    return res;
}



// This assumes we are trying to build a recombination history
int ComputeBlockHistoryLB( const BinaryMatrix &matInput, int nFounders )
{
    int res = 0;

    // the idea is to compute for each interval, the minimum number of 
    // crossover esimated
    map<INTERVAL, int> mapIntervalBds;

    for(int ivLeft = 0; ivLeft < matInput.GetColNum(); ++ivLeft )
    {
        for(int ivRight = ivLeft + 1; ivRight < matInput.GetColNum(); ++ivRight)
        {
//cout << "ivLeft = " << ivLeft << ", ivRight = " << ivRight << endl;
            // now we want to find out how much of it
            BinaryMatrix submat;
            matInput.SubMatrix( 0, matInput.GetRowNum()-1, ivLeft, ivRight, submat );

            // remove duplicate rows here
            submat.TrimDupRows();
            int localBd = 0;
            if( submat.GetRowNum() > nFounders )
            {
                localBd = submat.GetRowNum() - nFounders ;
            }

//cout << "Local bd = " << localBd << endl;
            if( localBd > 0 )
            {
                // Now add this entry to local bound
                INTERVAL iv( ivLeft, ivRight);
                mapIntervalBds.insert( map<INTERVAL, int> :: value_type( iv, localBd ) );
            }
        }
    }

    // Nnow this is the global bound
    vector<int> localBreakpoints;
    res = CalcCompositeBound( mapIntervalBds, 0, matInput.GetColNum()-1, localBreakpoints);


    return res;
}



// This is a very useful function, so expose it
int CheckSeqForFounderFrom( const FOUNDER_LIST &founder, const SEQUENCE &seq, int rowStart,
                                  set<int> &endRows)
{
    // Return the number of crossovers
    // This function computes the minimum recombination weight for the given hapRow
    // when restricted to interval [left, right] in mat
    int res = 0;

    set<int> lastTrackRows;  // set of rows that matching the hapRow

    // If we ware intereested in a particular row to start, 
    if( rowStart >= 0 )
    {
        lastTrackRows.insert( rowStart );
    }
    else
    {
        // Ohterwise, we can start from all possible rows
        for( unsigned int i=0; i<founder.size(); ++i )
        {
            lastTrackRows.insert( i );
        }
    }

    int left = 0; 
    int right = seq.size()-1;
    for(int curpos=left; curpos <=right; ++curpos)
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
        if( trackRows.size() == 0 )
        {
            // In this case, we can not build sequences by recombiinations only. Mutations are needed here
            return -1;
        }

        // Now we test if there is intersection, if non-empty, we contiinue
        set<int> sint;
        JoinSets(trackRows, lastTrackRows, sint);
        if(sint.size() == 0)
        {
            // No intersection, so we have to increase the result (we know there must be one recomb
            // here, from the right-maximal proof)
            ++res;

            // Re-initialize lastTrackRows here
            lastTrackRows = trackRows;
//            PopulateSetWithInterval( lastTrackRows, 0, mat.size() - 1 );
        }
        else
        {
            // In this case, we still continue
            lastTrackRows = sint;
        }
    }

    endRows = lastTrackRows;

    return res;
}

// This is a very useful function, so expose it
int DerSeqFromFounderMut( FOUNDER_LIST &founder, const SEQUENCE &seq )
{
    YW_ASSERT_INFO( founder.size() > 0 && founder[0].size() >0, "Founders can not be empty" );
    // This funcgtion alows mutation. So we first find out how much uniform sites there are
    vector<int> mutSites;
    FindMutSitesForFounders( founder, seq, mutSites );    

    // Now we mutate it
    SEQUENCE mutseq = seq;
    MutateSequenceAtSites( mutseq, mutSites );
    set<int> emptySet;
    return CheckSeqForFounderFrom(founder, mutseq, -1, emptySet) + mutSites.size();
}

int DumpParseSeqByFounders( const FOUNDER_LIST &founder, const SEQUENCE &seq )
{
    // This function is similar to CheckSeqForFounderFrom, but just dump out a parse for the seq
    set<int> lastTrackRows;  // set of rows that matching the hapRow
    int res = 0;

    // If we ware intereested in a particular row to start, 
    // Ohterwise, we can start from all possible rows
    for( unsigned int i=0; i<founder.size(); ++i )
    {
        lastTrackRows.insert( i );
    }

    int left = 0; 
    int right = seq.size()-1;
    int firstCharToOutput = 0;
    for(int curpos=left; curpos <=right; ++curpos)
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
        YW_ASSERT_INFO( trackRows.size() > 0, "Can not parse it." );

        // Now we test if there is intersection, if non-empty, we contiinue
        set<int> sint;
        JoinSets(trackRows, lastTrackRows, sint);
        if(sint.size() == 0)
        {
            // No intersection, so we dump the value here till hte last one
            for( int pp = firstCharToOutput; pp < curpos; ++pp )
            {
                char fc = 'A' + *lastTrackRows.begin();
                cout << fc;
            }

            // Update the firstCharToOutput
            firstCharToOutput = curpos;

            // Re-initialize lastTrackRows here
            lastTrackRows = trackRows;
//            PopulateSetWithInterval( lastTrackRows, 0, mat.size() - 1 );

            res ++;
        }
        else
        {
            // In this case, we still continue
            lastTrackRows = sint;
        }
    }
    // we output the final patch of it
    for(int pp= firstCharToOutput; pp <(int)seq.size(); ++pp)
    {
        char fc = 'A' + *lastTrackRows.begin();
        cout << fc;
    }
    cout << endl;

    return res;
}

void DumpParseMatrixByFounders( const FOUNDER_LIST &founder, const BinaryMatrix &inputMat )
{
    int totBkpts = 0;
    for(int r=0; r<inputMat.GetRowNum(); ++r)
    {
        SEQUENCE row;
        inputMat.GetRow( r, row );
        totBkpts += DumpParseSeqByFounders( founder, row );
    }
    cout << "Total number of breakpoints = " << totBkpts << endl;
}

void GetFounderState(FOUNDER_LIST &founder, const SEQUENCE &seq, vector<int> &founderBits)
{
    founderBits.clear();

    set<int> lastTrackRows;  // set of rows that matching the hapRow

    // If we ware intereested in a particular row to start, 
    // Ohterwise, we can start from all possible rows
    for( unsigned int i=0; i<founder.size(); ++i )
    {
        lastTrackRows.insert( i );
    }

    int left = 0; 
    int right = seq.size()-1;
    int bitToProcess = 0;
    for(int curpos=left; curpos <=right; ++curpos)
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
            // No intersection, so we have to increase the result (we know there must be one recomb
            // here, from the right-maximal proof)
            // Now we need to update all the way the bit just before this
            for(int bb = bitToProcess; bb < curpos; ++bb)
            {
                // simply take the first one available
                YW_ASSERT_INFO( lastTrackRows.size() > 0, "Must have some info." );
                founderBits.push_back( *lastTrackRows.begin()  );
            }
            bitToProcess = curpos;


            // Re-initialize lastTrackRows here
            lastTrackRows = trackRows;

        }
        else
        {
            // In this case, we still continue
            lastTrackRows = sint;
        }
    }
    // we finish the lest over bits
    for( int bb = bitToProcess; bb < (int) seq.size(); ++bb) 
    {
        YW_ASSERT_INFO( lastTrackRows.size() > 0, "Must have some info." );
        founderBits.push_back( *lastTrackRows.begin()  );
    }
    YW_ASSERT_INFO(founderBits.size() == seq.size() , "Not enough bits");
}




// This is for test code only
void ExamWndUniqueness( int wndSize, int nFounders )
{
    YW_ASSERT_INFO(nFounders >= 2, "Must have at least 2 founders" );

    int cnt = 0;

    // This function simply exhaustive search all possible window
    // with certain number of SNPs. The goal is to find whether 
    // there are instances of founders s.t. the input/output
    // is still correlated. This independent property is
    // useful in designing efficient algorithms

    // We simply enumerate all possible way of founders for certain window size
    // We require there at least one founders to be 0 and 1 at any site
    // For now, we do not allow neighbouring founder sites to be identical or complementary
    vector< FOUNDER_LIST > founderCandidates;


    // 
    ConsFounderRec( 0, wndSize, nFounders, founderCandidates);
#if 0
    // dump the founders to make sure they are correct
    for(unsigned int i=0; i<founderCandidates.size(); ++i)
    {
        if( IsFounderValid(founderCandidates[i], wndSize, nFounders) == false )
        {
            // not a valid one
            continue;
        }

        cout << "Founder #" << i  << ": \n";
        for(unsigned int j=0; j<founderCandidates[i].size(); ++j)
        {
            DumpSequence( founderCandidates[i][j]);
        }
    }
#endif




    // Now we have constructed set of all possible sequences with the given length
//    for(int n = 0; n < (0x1 << wndSize); ++n)
    int n=0;    // i think we only need to try one sequence, say all-0 here since we are testing for all founders
    {
        SEQUENCE seq;
        GetHyperCubeSeq(n, seq, wndSize);
    
        // Now for each founder, we make sure this found is good for it
        for(unsigned int j=0; j<founderCandidates.size(); ++j)
        {
//cout << "Testing founder j=" << j << endl;
            if( IsFounderValid(founderCandidates[j], wndSize, nFounders) == false )
            {
//cout << "bad founder.\n";
                // not a valid one
                continue;
            }
#if 0
            // we also make sure there are no identical rows
            if ( IsFounderWithIdentRows( founderCandidates[j], wndSize, nFounders ) == true )
            {
                continue;
            }
#endif

            // assume seq does not match exactly part of the founder
#if 0
            if ( IsSeqMatchFounder (seq,  founderCandidates[j]  ) == true )
            {
                continue;
            }
#endif
#if 0
            if( CheckSeqForFounder( founderCandidates[j],  seq ) == false )
            {
cout << "This founder is bad for seq = ";
DumpSequence( seq);
cout << "Founder = \n";
for(unsigned int k=0; k<founderCandidates[j].size(); ++k)
{
    DumpSequence( founderCandidates[j][k]);
}
//                exit( 1 );
            }
#endif
            if( CheckSeqForFounderExitStatus( founderCandidates[j],  seq ) == false )
            {
                cnt ++;
            }

        }
    }
    cout << "Total number of failed configuraitons = " << cnt << endl;
}

// This function computes the number of pathways deriving the given sequence from a set of founders
// and assuming the number of events in the pathway is bounned by some upper bounds
double CalcNumDerFromFounders( const FOUNDER_LIST &founders, const SEQUENCE &seqTarget, int numEvents )
{
    // We first figure out the mutations needed
    YW_ASSERT_INFO( founders.size() > 0 && founders[0].size() >0, "Founders can not be empty" );
    YW_ASSERT_INFO( founders[0].size() == seqTarget.size(), "Founders and seq mismatch" );
    
    vector<int> mutSites;
    FindMutSitesForFounders( founders, seqTarget, mutSites );    

    // Now we mutate it
    SEQUENCE mutseq = seqTarget;
    MutateSequenceAtSites( mutseq, mutSites );

    if( (int)mutSites.size() > numEvents )
    {
        // Can not derive with specified events
        return -1;
    }

    // Now we find how many segments we need by dynamic programming
    int numCrossovers = numEvents - mutSites.size();

    // Now start dynamic programming
    int dataLen = seqTarget.size();
    // N[c,r,k]: r is row, c is site for column dataLen-c-1, k is num events
    vector< vector< vector<double> > > DPTable;
    for( int c=0; c<dataLen; ++c)
    {
        vector< vector<double> > entry;
        for( unsigned int r=0; r<founders.size(); ++r)
        {
            vector<double> entryEvents;
            for( int k=0; k<=numCrossovers; ++k  )
            {
                entryEvents.push_back( 0.0 );
            }
            entry.push_back( entryEvents );
        }
        DPTable.push_back( entry );
    }

    // Now we start the DP procedure by initialize the c=1 case
    int initCol = dataLen -1;
    int numFounders = founders.size();
    for(  int r=0; r<numFounders; ++r)
    {
        // If the row matches the seq, then num = 1
        if( mutseq[initCol] == founders[r][initCol] )
        {
            // Then this is 1
            DPTable[0][r][0] = 1.0;
            for( int k=1; k<=numCrossovers; ++k  )
            {
                DPTable[0][r][k] = 1.0;
            }

        }
        else
        {
            // If does not match, we can not achive it using 0 event
            DPTable[0][r][0] = 0.0;
            for( int k=1; k<=numCrossovers; ++k  )
            {
                DPTable[0][r][k] = 0.0;
            }
       }
    }

    // Now handle by increasing c
    for(int c=1; c<dataLen; ++c)
    {
        int realCol = dataLen - c - 1;
        for(  int r=0; r<numFounders; ++r)
        {
            if( mutseq[realCol] == founders[r][realCol] )
            {
                // This mutated seq matches founders at the position
                for( int k=0; k<=numCrossovers; ++k  )
                {
                    DPTable[c][r][k] = DPTable[c-1][r][k];
                    if( k== 0)
                    {
                        continue;
                    }
                    for( int r1=0; r1<numFounders; ++r1 )
                    {
                        int realCol1 = realCol+1;
                        if( mutseq[realCol1] != founders[r1][realCol1]  )
                        {
                            continue;
                        }
                        DPTable[c][r][k]  += DPTable[c-1][r1][k-1] ;
                    }
                }
            }
            else
            {

                // Mismatch here, then we must only try others
                // skip it
#if 0
                // this cell remains 0 when k=0
                for( int k=1; k<=numCrossovers; ++k  )
                {
                    for( int r1=0; r1<numFounders; ++r1 )
                    {
                        int realCol1 = realCol+1;
                        if( mutseq[realCol1] != founders[r1][realCol1]  )
                        {
                            continue;
                        }
                        DPTable[c][r][k]  += DPTable[c-1][r1][k-1] ;
                    }
                }
#endif
            }
        }
        
    }
#if 0
cout << "Founder derivation table = \n";
for( int c=0; c<dataLen; ++c)
{
cout << "realpos = " << dataLen - c - 1 << endl;
for( unsigned int r=0; r<founders.size(); ++r)
{
cout << "Founder row = " << r << endl;
DumpIntVec( DPTable[c][r] );
}
}
#endif

    // The results = sum over all possible rows
    double res = 0;
    for( int r=0; r<numFounders; ++r )
    {
        res += DPTable[dataLen-1][r][numCrossovers];
    }
//cout << "res = " << res << endl;
    return res;
}

// This function derives a set of sequences from founders
// assume no mutations 
int DerMatrixFromFounder(  const FOUNDER_LIST &founders, const BinaryMatrix &mat)
{
    // make sure the ddata size makes sense
    YW_ASSERT_INFO( founders.size() > 0 && (int)founders[0].size() == mat.GetColNum(), "Founder wrong." );
    int res = 0;
    // derive one row by another
    for( int r=0; r<mat.GetRowNum(); ++r )
    {
        SEQUENCE seq;
        mat.GetRow(r, seq);
        set<int> exitRows;
        int stepRes = CheckSeqForFounderFrom( founders, seq, -1, exitRows );
        YW_ASSERT_INFO( stepRes >= 0, "Can not be negative." );
        res += stepRes;
    }
    return res;
}


// This function computes two founder problem with the simple trick
int CalcTwoFounderMinBP(const BinaryMatrix &matInput)
{
#if 0
    // record the decision we make along the way
    vector<bool> listDecisions( matInput.GetColNum()  );

    // assume no missing vlaue here
    int res = 0;
    bool fMiss = matInput.IsMissingValue();
    for(int c=1; c < matInput.GetColNum(); ++c)
    {
        // get the smaller of 00/11 and 01/10
        int n0011 = 0, n0110 = 0;
        for(int r =0; r<matInput.GetRowNum(); ++r)
        {
            if()
            {
            }
            else
            {
            }
        }

        // 
        if( n0011 <
    }
    return res;
#endif
    return -1;  // TBD, do not know how to make it work for now
}

// This function tests our solution for missing value problem
int TestTwoFounderMVMethod( const BinaryMatrix &matMiss )
{
    // delete any potentially uniform sites

    // now enumreate all possible settings of missing values

    // compare with the true ones

    return -1; // TBD
}


