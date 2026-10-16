#include "Utils.h"
#include "Utils2.h"
#include "ctime"
#include "cstdlib"
#include "cstdio"

/////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////////////////////

long GetCurrentTimeTick()
{
    return (long) time(NULL);
}

long GetElapseTime(long lastTime)
{
    return (long) (   (long) time(NULL)  - lastTime) ; 
}

bool IsBoolArrayAllTrue( bool *bArray, int size)
{
    for(int i=0; i<size; ++i)
    {
        if( bArray[i] == false)
        {
            return false;
        }
    }
    return true;
}

void AppendIntVec(vector<int> &dest, const vector<int> &appending)
{
    for(unsigned int i=0; i<appending.size(); ++i)
    {
        dest.push_back( appending[i] );
    }
}


bool IsSetContainer(const set<int> &container, const set<int> &contained)
{
    //
    if( container.size() < contained.size()   )
    {
       // clearly not true
       return false;
    }
    //
    for(set<int> ::iterator it = contained.begin(); it != contained.end(); ++it)
    {
        if(  container.find( *it ) == container.end()  )
        {
            return false;
        }
    }
    return true;
}

bool IsSetContainedInSets( const set<int> &s, const set< set<int> > &sets)
{
    // This function return true if ONE of the sets in sets contains s, false otherwise
    for( set< set<int> > :: iterator it = sets.begin(); it != sets.end(); ++it )
    {
        if(  IsSetContainer( *it,  s ) == true )
        {
            return true;
        }
    }
    return false;
}

bool IsSetContainingOneOfSets( const set<int> &s, const set< set<int> > &sets )
{
    // This function return true if ONE of the sets in sets contains s, false otherwise
    for( set< set<int> > :: iterator it = sets.begin(); it != sets.end(); ++it )
    {
        if(  IsSetContainer( s,  *it ) == true )
        {
            return true;
        }
    }
    return false;
}


void ConcatIntVec( vector<int> &vecAdded, const vector<int> & vecToAdd)
{
    // Append one vector to another
    for(unsigned int i=0; i < vecToAdd.size(); i++)
    {
        vecAdded.push_back( vecToAdd[i] );
    }
}

int ConvIntSetToPosition( const set<int> &s  )
{
//cout << "In ConvIntSetToPosition: s = ";
//DumpIntSet( s );
    // this function convert an integer set to a position index, for example,
    // if range = 8, s={2, 4}, then this converts to 00010100
    int res = 0;
    for( set<int> :: iterator it = s.begin(); it != s.end(); ++it)
    {
        int a = *it;
        int mask = 0x1 << a;
        res = res | mask;
    }
//cout << "conversion res = " << res << endl;
    return res;
}

void PopulateSetWithInterval( set<int> &s, int left, int right)
{
    s.clear();
    for(int i = left; i<=right; ++i)
    {
        s.insert( i );
    }
}

void GetSeqInterval(const SEQUENCE &row, SEQUENCE  &rowIV, int left, int right)
{
    rowIV.clear();
    for(int i=left; i<=right; ++i)
    {
        rowIV.push_back( row[i] );
    }
}

bool IsIntervalContained( const set<SEQUENCE> &seqs, int left, int right, const SEQUENCE &seqIV )
{
    // This function check to see if the seqIV is contained in the seqs
    for(set<SEQUENCE> :: iterator it = seqs.begin(); it != seqs.end(); ++it )  
    {
        SEQUENCE substr;
        GetSeqInterval( *it, substr, left, right);
        if( substr == seqIV )
        {
            return true;
        }
    }
    return false;
}


void SubtractSequenceSets( set<SEQUENCE> &s1, const set<SEQUENCE> &s2)
{
    if( s2.size() == 0)
    {
        return;
    }
    set<SEQUENCE> res;
	// this function performs set intersection, i.e. s1=s1 ^s2
	for( set<SEQUENCE>::iterator it = s1.begin(); it!= s1.end(); ++it)
	{
		if( s2.find( *it ) == s2.end() )
		{
			res.insert( *it ); 
		}
	}
    s1.clear();
    s1 = res;
}

void DumpSequence( const SEQUENCE &seq)
{
    for(unsigned int i=0; i<seq.size(); ++i)
    {
        cout << seq[i];
    }
    cout << endl;
}

void DumpSetSequences( const set<SEQUENCE> &setSeqs )
{
    for( set<SEQUENCE> :: iterator it = setSeqs.begin(); it != setSeqs.end(); ++it)
    {
        DumpSequence( *it );
    }
}


bool AreTwoInSameSet(int i1, int i2, const set< set<int> > &collections)
{
    // Check to see if i1 and i2 is in same set
    for( set< set<int> > :: iterator it = collections.begin(); it != collections.end(); ++it)
    {
        bool found1 = false, found2 = false;
        if( it->find( i1) != it->end() )
        {
            found1 = true;
        }
        if( it->find( i2 ) != it->end() )
        {
            found2 = true;
        }
        if( found1 == true && found2 == true)
        {
//cout << "i1 = " << i1 << ", i2 = " << i2 << " INDDED in same set.\n";
            return true;
        }
        if( found1 || found2)
        {
//cout << "i1 = " << i1 << ", i2 = " << i2 << " not in same set.\n";
            return false;
        }
    }
    // should not need this, in case
    YW_ASSERT_INFO( false, "Bad i1 or i2." );
    return false;
}

int GetItemIndexInVec( const vector<int> &vec, int item)
{
    for(unsigned int i=0; i<vec.size(); ++i)
    {
        if( vec[i] == item )
        {
            return i;
        }
    }
    return -1;
}

bool IsIntervalOverlap( const INTERVAL &iv1, const INTERVAL &iv2)
{
    if(  iv1.second < iv2.first || iv2.second < iv1.first  )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool GetIntervalOverlap( const INTERVAL &iv1, const INTERVAL &iv2, INTERVAL &ivBoth)
{
    int left = iv1.first;
    if( left < iv2.first)
    {
        left = iv2.first;
    }
    int right = iv1.second;
    if( right > iv2.second)
    {
        right = iv2.second;
    }
    if( left > right )
    {
        return false;
    }
    else
    {
        ivBoth.first = left;
        ivBoth.second = right;
        return true;
    }
}


void GenerateRandBinVector(int sz, vector<int> &randVec)
{
//cout << "GenerateRandBinVector: sz = " << sz << endl;
    // Generate random vector
    randVec.clear();
    for(int i=0; i<sz; ++i)
    {
//cout << " i = " << i << endl;
        double r = GetRandFraction();
        if( r >= 0.5)
        {
            randVec.push_back( 0 );
        }
        else
        {
            randVec.push_back( 1 );
        }
    }
}
bool IsBinary( int val)
{
    if( val == 0 || val == 1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

int PickRandNumFromList( vector<double> &listNums)
{
    if( listNums.size() == 0 )
    {
        // no work to do
        return 0;
    }

    // This function returns a randomly picked indices from 
    // an array, weighted by the numbers in the array
    // First got to know the total value of all numbers
    double totalNum = 0.0;
    for( unsigned int i=0; i<listNums.size(); ++i)
    {
        totalNum += listNums[i];
    }

    double randVal = GetRandFraction();
    double curVal = 0.0;
    for(unsigned int i=0; i<listNums.size(); ++i)
    {
        curVal += listNums[i];
        if( curVal > totalNum * randVal )
        {
            return i;
        }
    }
    // if reach here, return the last one
    return listNums.size() - 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
bool IsSequenceHaplotype( const SEQUENCE &seq)
{
    for(unsigned int i = 0; i < seq.size(); ++i)
    {
        if( seq[i] != 0 && seq[i] != 1)
        {
            return false;
        }
    }
    return true;
}

bool IsSequenceGenotype( const SEQUENCE &seq)
{
    for(unsigned int i = 0; i < seq.size(); ++i)
    {
        if( seq[i] != 0 && seq[i] != 1 && seq[i] != 2)
        {
            return false;
        }
    }
    return true;
}

bool CanPhaseGenoRow( const SEQUENCE &hap1, const SEQUENCE &hap2, const SEQUENCE &geno )
{
    YW_ASSERT_INFO(  IsSequenceHaplotype(hap1), "hap1 is not haplotype row."  );
    YW_ASSERT_INFO(  IsSequenceHaplotype(hap2), "hap1 is not haplotype row."  );
    YW_ASSERT_INFO(  IsSequenceGenotype(geno), "hap1 is not haplotype row."  );
    YW_ASSERT_INFO( hap1.size() == hap2.size(), "Tow hap rows are not equal length");
    YW_ASSERT_INFO( geno.size() == geno.size(), "Geno row is not the same size as hap row." );
//cout << "hap1 = ";
//DumpIntVec(hap1 );
//cout << "hap2 = ";
//DumpIntVec(hap2 );
//cout << "geno = ";
//DumpIntVec( geno );
    for(unsigned int i=0; i<hap1.size(); i++)
    {
        if(  geno[i] == 2 )
        {
            if( hap1[i] + hap2[i] != 1  )
            {
                return false;
            }
        }
        else 
        {
            if( hap1[i] + hap2[i] != 2*geno[i]  )
            {
                return false;
            }
        }
    }
    return true;
}

bool AreHapGenoRowCompatible( const SEQUENCE &hapRow, const SEQUENCE &genoRow, SEQUENCE *pComplement)
{
    if( pComplement != NULL)
    {
        pComplement->clear();
    }

    // Check if the haplotype row can be a phasing of the geno row
    YW_ASSERT_INFO(  IsSequenceHaplotype(hapRow), "hap is not haplotype row."  );
    YW_ASSERT_INFO(  IsSequenceGenotype(genoRow), "genorow is not haplotype row."  );
    for(unsigned int i=0; i<hapRow.size(); i++)
    {
        if(  genoRow[i] != 2 )
        {
            if( hapRow[i] != genoRow[i]  )
            {
                return false;
            }
            else
            {
                if( pComplement != NULL)
                {
                    pComplement->push_back(  genoRow[i]  );
                }
            }
        }
        else
        {
            if( pComplement != NULL)
            {
                if( hapRow[i] == 0 )
                {
                    pComplement->push_back(  1  );
                }
                else
                {
                    pComplement->push_back(  0  );
                }
            }
        }
    }
    return true; 
}

bool AreHapGenoRowsSame( const SEQUENCE &hapRow, const SEQUENCE &genoRow  )
{
    YW_ASSERT_INFO(  IsSequenceHaplotype(hapRow), "hap is not haplotype row."  );
    YW_ASSERT_INFO(  IsSequenceGenotype(genoRow), "genorow is not haplotype row."  );
    return hapRow == genoRow;
}

bool IsTrivialRow( const SEQUENCE &row, SEQUENCE &resolved1, SEQUENCE &resolved2 )
{
    resolved1.clear();
    resolved2.clear();
    // A row is trivial if it contains only a single 2
    YW_ASSERT_INFO(  IsSequenceGenotype(row), "hap is not haplotype row."  );
    int num2s = 0;
    for(unsigned int i=0; i<row.size(); ++i)
    {
        if( row[i] == 2 )
        {
            ++num2s;
            if( num2s > 1 )
            {
                break;
            }
        }
        if( row[i] == 2 )
        {
            resolved1.push_back( 0 );
            resolved2.push_back( 1 );
        }
        else
        {
            resolved1.push_back( row[i] );
            resolved2.push_back( row[i] );
        }
    }
    if( num2s == 1 )
    {
        // For now, we do not consider a row with no twos as new
        return true;
    }
    else
    {
        return false;
    }
}

bool IsHapSeqSmaller( const SEQUENCE &hapRow1, const SEQUENCE &hapRow2)
{
    // Decide whether hapRow1 is smaller
    // used in situations when we need to compare two rows
    YW_ASSERT_INFO(  IsSequenceHaplotype(hapRow1), "hap1 is not haplotype row."  );
    YW_ASSERT_INFO(  IsSequenceHaplotype(hapRow2), "hap2 is not haplotype row."  );
    YW_ASSERT_INFO( hapRow1.size() == hapRow2.size(), "Tow hap rows are not equal length");
    for(unsigned int i=0; i<hapRow1.size(); ++i)
    {
        if( hapRow1[i] < hapRow2[i])
        {
            return true;
        }
    }
    return false;
}


void GetHyperCubeSeq(int hcSeq, SEQUENCE &seq, int hcWidth)
{
    ConvIntToVecMSB(hcSeq, seq, hcWidth );
}

int GetSeqIdFromSeq( const SEQUENCE &seq )
{
    return ConvVecToIntMSB( seq );
}

int GetHyperCubSeqBit( int hcSeq, int bit, int hcWidth)
{
    // Retrive the bit in the hcSeq at the specified bit
    // But  note that we are assuming bit 0 is on the left (BIG ENDIAN)
    int shiftpos = hcWidth - bit - 1;
    int mask = 0x1 << shiftpos;
    int res =  (hcSeq & mask) >> shiftpos;
    YW_ASSERT_INFO( res == 0 || res == 1, "Serious error here." );
    return res;
}


void FindNonSegSites( const set<HCSequence> &setSeqs, set<int> &sites, int dataWidth )
{
    // Now we find out whe mutation sites that are not used in the setSeqs
    // Find the set of sites that are not segragating in the set of sequences 
    for(int i=0; i<dataWidth; ++i)
    {
        bool fZero = false, fOne = false;
        // Check to see if site i is segragating or not
        for(  set<int> :: iterator it = setSeqs.begin(); it != setSeqs.end(); ++it  )
        {
            int rn = *it;
//cout <<"In FindNonSegragateSites: rn = " << rn << endl;
            if(  GetHyperCubSeqBit(rn,  i, dataWidth )  == 0 )
            {
                fZero = true;
            }
            else
            {
                fOne = true;
            }
            if( fZero && fOne)
            {
                break;
            }
        }
        if( fZero == false || fOne == false)
        {
            sites.insert( i );
        }
    }

}

void FindNonSegSites( const set<SEQUENCE> &setSeqs, set<int> &sites, int dataLen )
{
    sites.clear();
    if( setSeqs.size() == 0 )
    {
        // Every one is non-segragating
        for(int i=0; i<dataLen; ++i)
        {
            sites.insert( i );
        }
        return;
    }

    for(int i=0; i<dataLen; ++i)
    {
        bool fZero = false, fOne = false;
        // Check to see if site i is segragating or not
        for(  set<SEQUENCE> :: iterator it = setSeqs.begin(); it != setSeqs.end(); ++it  )
        {
            SEQUENCE row = *it;
//cout <<"In FindNonSegragateSites: rn = " << rn << endl;
            if(  row[ i ]  == 0 )
            {
                fZero = true;
            }
            else
            {
                fOne = true;
            }
            if( fZero && fOne)
            {
                break;
            }
        }
        if( fZero == false || fOne == false)
        {
            sites.insert( i );
        }
    }
}


void CreateGenoRowFromHapRows( const SEQUENCE &hapRow1, const SEQUENCE &hapRow2,  SEQUENCE &genoRow)
{
    // Check if the haplotype row can be a phasing of the geno row
    YW_ASSERT_INFO(  IsSequenceHaplotype(hapRow1), "hap1 is not haplotype row."  );
    YW_ASSERT_INFO(  IsSequenceHaplotype(hapRow2), "hap2 is not haplotype row."  );
    genoRow.clear();
    for(unsigned int i=0; i<hapRow1.size(); i++)
    {
        if(  hapRow1[i] == hapRow2[i]  )
        {
            genoRow.push_back( hapRow1[i] );
        }
        else
        {
            genoRow.push_back( 2 );
        }
    }
}


int IsHCSeqsMutPair(  HCSequence seq1, HCSequence seq2, int dataWidth)
{
    // This function test if seq1/seq2 is mutation pair, and return the mut site if so
    for(int p = 0; p< dataWidth; p++)
    {
        int shiftpos = dataWidth - p - 1;
        int mask = 0x1 << shiftpos;

        if( (seq1 | mask) == (seq2 | mask) )
        {
            return p;
        }
    }

    return -1;      // indicate NOT-pair
}


bool IsHCSeqsMutPairAt(  HCSequence seq1, HCSequence seq2, int dataWidth, int pos)
{
    // Different from the previous function, this check for a specific location
    // instead of trying all possible positions
    int shiftpos = dataWidth - pos - 1;
    int mask = 0x1 << shiftpos;

    if( (seq1 | mask) == (seq2 | mask) )
    {
        return true;
    }
    return false;
}

void MutateHCSeqAt( const HCSequence seq, HCSequence & res, int dataWidth, int mutPos )
{
    int shiftpos = dataWidth - mutPos - 1;
    int mask = 0x1 << shiftpos;

    res = (seq ^ mask);

}



bool IsHCSeqRecombinnable( HCSequence hcSeq1, HCSequence hcSeq2, HCSequence st, int dataWidth) 
{
    // ASSUME: s1 is LEFT part and s2 is RIGHT part
    // This function test if s1 and s2 can recombine into st
    // Now start recombining
    for(int bkpt = 0; bkpt < dataWidth-1; ++bkpt)
    {
        unsigned int maskLower = (0x1 << (bkpt+1) ) - 1;
        unsigned int maskUpper = (0x1 << dataWidth ) - 1 - maskLower ;

        // Generate s sequence
        int seq1 = ((hcSeq1 & maskLower)  | (hcSeq2 & maskUpper) );
        if( seq1 == st)
        {
            return true;
        }
#if 0       // here we
        int seq2 = ((hcSeq1 & maskUpper)  | (hcSeq2 & maskLower) );
        if( seq2 == st)
        {
            return true;
        }
#endif
    }

    return false;
}


void RecombineHCSeqs( const HCSequence hcSeq1, const HCSequence hcSeq2, HCSequence &res, int dataWidth, int bkpt) 
{
    // ASSUME: s1 is LEFT part and s2 is RIGHT part
    // This function test if s1 and s2 can recombine into st
    // Now start recombining
    unsigned int maskLower = (0x1 << (bkpt+1) ) - 1;
    unsigned int maskUpper = (0x1 << dataWidth ) - 1 - maskLower ;

    // Generate s sequence
    res = ((hcSeq1 & maskLower)  | (hcSeq2 & maskUpper) );

}
