#include "GenotypeMatrix.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "Utils.h"
#include "Utils2.h"

// ***************************************************************************
// Define a reusable binary matrix class
// ***************************************************************************

GenotypeMatrix :: GenotypeMatrix() 
{
    nCols = 0;
}

GenotypeMatrix :: ~GenotypeMatrix()
{
	// Need to free up data if needed
	Clear(); 
}			   

GenotypeMatrix :: GenotypeMatrix(int nr, int nc)
{
	SetSize( nr, nc ); 
}


GenotypeMatrix :: GenotypeMatrix( const GenotypeMatrix &rhs)
{
	Copy(rhs); 
}


GenotypeMatrix & GenotypeMatrix :: operator=(const GenotypeMatrix &rhs)
{
	Clear(); 

	Copy(rhs); 

	return *this; 
}

bool GenotypeMatrix :: IsDataValid(int val) 
{
    if( val == 0 || val == 1 || val == 2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GenotypeMatrix :: PreSolve()
{
    // Generate the companion rows
    SetupCompanionColumns();
}

bool GenotypeMatrix :: AreColumnsCompanion(int c1, int c2) 
{
    if( c1 == c2)
    {
        return false;
    }
    if(c1 > c2)
    {
        int tmp = c1;
        c1 = c2;
        c2 = tmp;
    }
    COLUMN_PAIR cp(c1,c2);
    if( companionRows.find(cp) == companionRows.end() || companionRows[cp].size() == 0 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool GenotypeMatrix :: AreColumnsForcedInPhase(int c1, int c2) 
{
    if( c1 == c2)
    {
        return false;
    }
    if(c1 > c2)
    {
        int tmp = c1;
        c1 = c2;
        c2 = tmp;
    }
    COLUMN_PAIR cp(c1,c2);
    if( forcedColumnPairs.find(cp) == forcedColumnPairs.end() || forcedColumnPairs[cp] == 1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool GenotypeMatrix :: AreColumnsForcedOutPhase(int c1, int c2) 
{
    if( c1 == c2)
    {
        return false;
    }
    if(c1 > c2)
    {
        int tmp = c1;
        c1 = c2;
        c2 = tmp;
    }
    COLUMN_PAIR cp(c1,c2);
    if( forcedColumnPairs.find(cp) == forcedColumnPairs.end() || forcedColumnPairs[cp] == 0 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool GenotypeMatrix :: AreColumnsComplete(int c1, int c2) const
{
    if( c1 == c2)
    {
        return false;
    }
    if(c1 > c2)
    {
        int tmp = c1;
        c1 = c2;
        c2 = tmp;
    }
    
    COLUMN_PAIR cp(c1,c2);
    for(unsigned int i=0; i<completePairs.size(); ++i)
    {
        if( completePairs[i] == cp )
        {
            return true;
        }
    }
    return false;
}

bool GenotypeMatrix :: AreColumnsCompleteDirect(int c1, int c2) const
{
    // Test whether the two columns are complete
    bool found00 = false, found01 = false, found10 = false, found11 = false;

    for(int k=0; k<GetRowNum(); ++k)
    {
        if( rowsArray[k][c1] == 0 && rowsArray[k][c2] == 2 ) // Now check for forced pattern
        {
            found00 = true;
            found01 = true;
        }
        else if( rowsArray[k][c1] == 1 && rowsArray[k][c2] == 2 )
        {
            found10 = true;
            found11 = true;
        }
        else if( rowsArray[k][c1] == 2 && rowsArray[k][c2] == 0 )
        {
            found00 = true;
            found10 = true;
        }
        else if( rowsArray[k][c1] == 2 && rowsArray[k][c2] == 1 )
        {
            found01 = true;
            found11 = true;
        }
        else if( rowsArray[k][c1] == 0 && rowsArray[k][c2] == 0 )
        {
            found00 = true;
        }
        else if( rowsArray[k][c1] == 0 && rowsArray[k][c2] == 1 )
        {
            found01 = true;
        }
        else if( rowsArray[k][c1] == 1 && rowsArray[k][c2] == 0 )
        {
            found10 = true;
        }
        else if( rowsArray[k][c1] == 1 && rowsArray[k][c2] == 1 )
        {
            found11 = true;
        }
    }
    if( found00 == true && found11 == true && found01 == true && found10 == true  )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool GenotypeMatrix :: AreColumnsPotentialIncompatible( int c1, int c2) const
{
//    if( AreColumnsComplete(c1,c2) ||  AreColumnsForcedInPhase(c1,c2) || AreColumnsForcedOutPhase(c1,c2)  )
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }

    // Test whether the two columns are complete
    bool found00 = false, found01 = false, found10 = false, found11 = false, found22= false;

    for(int k=0; k<GetRowNum(); ++k)
    {
        if( rowsArray[k][c1] == 0 && rowsArray[k][c2] == 2 ) // Now check for forced pattern
        {
            found00 = true;
            found01 = true;
        }
        else if( rowsArray[k][c1] == 1 && rowsArray[k][c2] == 2 )
        {
            found10 = true;
            found11 = true;
        }
        else if( rowsArray[k][c1] == 2 && rowsArray[k][c2] == 0 )
        {
            found00 = true;
            found10 = true;
        }
        else if( rowsArray[k][c1] == 2 && rowsArray[k][c2] == 1 )
        {
            found01 = true;
            found11 = true;
        }
        else if( rowsArray[k][c1] == 0 && rowsArray[k][c2] == 0 )
        {
            found00 = true;
        }
        else if( rowsArray[k][c1] == 0 && rowsArray[k][c2] == 1 )
        {
            found01 = true;
        }
        else if( rowsArray[k][c1] == 1 && rowsArray[k][c2] == 0 )
        {
            found10 = true;
        }
        else if( rowsArray[k][c1] == 1 && rowsArray[k][c2] == 1 )
        {
            found11 = true;
        }
        else if( rowsArray[k][c1] == 2 && rowsArray[k][c2] == 2 )
        {
            if(  found22 == false )
            {
                found22 = true;
            } 
//#if 0
            else
            {
                // If there are two 22, then we know they can be incompatible
                return true;
            }
//#endif
        }
    }
    if( found00 == true && found11 == true && found01 == true && found10 == true  )
    {
        return true;
    }

    // If there is complete pairs, then we must have either (00,11) or (01,10)
    if( found00 == true && found11 == true && found22 == true )
    {
        return true;
    }
    if( found01 == true && found10 == true && found22 == true )
    {
        return true;
    }

    
    return false;
}



int GenotypeMatrix :: GetNumTwosInRow(int r) const
{
    // For now, it is not optimized yet
    // we simply count the number of twos
    // later we can rely on preprocessing
    int res = 0;
    for(int i= 0; i<GetColNum(); ++i)
    {
        if( rowsArray[r][i] == 2 )
        {
            ++res;
        }
    }
    return res;
}

bool GenotypeMatrix :: IsSiteTrival( int site) const
{
    int numTwos = 0;
    int numZeros = 0;
    int numOnes = 0;
    for(int i=0; i<GetRowNum(); ++i)
    {
        if( rowsArray[i][site] == 0)
        {
            numZeros++;
        }
        else if(rowsArray[i][site] == 1)
        {
            numOnes++;
        }
        else if(rowsArray[i][site] == 2)
        {
            numTwos++;
        }
        else
        {
cout << "Error: matrix = ";
this->Dump();
            YW_ASSERT_INFO( false , "Impossible.");

        }
    }
    if( numTwos <= 1 && (numZeros == 0 || numOnes == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

int GenotypeMatrix :: GetNumZerosAtSite(int site) const
{
    int numTwos = 0;
    int numZeros = 0;
    int numOnes = 0;
    for(int i=0; i<GetRowNum(); ++i)
    {
        if( rowsArray[i][site] == 0)
        {
            numZeros++;
        }
        else if(rowsArray[i][site] == 1)
        {
            numOnes++;
        }
        else if(rowsArray[i][site] == 2)
        {
            numTwos++;
        }
        else
        {
cout << "Error: matrix = ";
this->Dump();
            YW_ASSERT_INFO( false , "Impossible.");

        }
    }
    return numZeros * 2 + numTwos;
}

void GenotypeMatrix :: GetStateRowsAtSite(int site, int state, vector<int> &rows) const
{
    rows.clear();
    for(int i=0; i<GetRowNum(); ++i)
    {
        if( rowsArray[i][site] == 0)
        {
            if( state == 0 )
            {
                rows.push_back( i );
            }
        }
        else if(rowsArray[i][site] == 1)
        {
            if( state == 1 )
            {
                rows.push_back( i );
            }
        }
        else if(rowsArray[i][site] == 2)
        {
            rows.push_back( i );
        }
        else
        {
cout << "Error: matrix = ";
this->Dump();
            YW_ASSERT_INFO( false , "Impossible.");

        }
    }
}


void GenotypeMatrix :: FindTrivialSites( set<int> &trivSites )
{
    // First find out which sites are trivial
    trivSites.clear();
//    set<int> trivialSites;
    for(int i=0; i<GetColNum(); ++i)
    {
        if( IsSiteTrival( i ) == true)
        {
//cout << "site " << i << " is trivial.\n";
            trivSites.insert( i );
        }
    }
    // Now dropping these sites
//    RemoveColumns( trivSites );
}

void GenotypeMatrix :: FindFullyCompatibleSites( set<int> &compatSites )
{
    // This funnction finds the set of sites whose setting of 2s always make
    // the site compatible with all other sites (thus, this includes those trivial sites )
    bool *flags = new bool[  GetColNum()  ];
    for(int i=0; i<GetColNum(); ++i)
    {
        flags[i] = false;
    }
    for(int s = 0; s< GetColNum(); ++s)
    {
        // Check whether site s can not be in conflict with any other sites
        for(int s2=s+1; s2<GetColNum(); ++s2)
        {
            if( AreColumnsPotentialIncompatible( s, s2 ) == true)
            {
                flags[ s ] = true;
                flags[ s2 ] = true;
            }
        }
    }
    // Now we only keep these sites whose flags are still false
    for(int i=0; i<GetColNum(); ++i)
    {
        if( flags[i] == false )
        {
            compatSites.insert( i );
        }
    }
    delete [] flags;
}


void GenotypeMatrix :: TrivialPartExpand()
{
    // Trivial expansion by removing rows with single 2 and replace with two haplotype rows
    set<SEQUENCE> seqsNew;
    set<int> rowsToRemove;
    for(int i=0; i<GetRowNum(); ++i)
    {
        SEQUENCE row;
        GetRow(i, row );
        SEQUENCE rn1, rn2;
        if( IsTrivialRow( row, rn1, rn2 ) == true)
        {
            rowsToRemove.insert( i );
            seqsNew.insert( rn1);
            seqsNew.insert( rn2);
        }
    }
    // Now we first remove these found rows
    RemoveRows( rowsToRemove );
    AppendSetOfRows( seqsNew );
}

// Test whether the haplotype row can be used in phasing of this genotype matrix
bool GenotypeMatrix :: IsHapSeqCompatible(const SEQUENCE &hapSeq) const
{
    YW_ASSERT_INFO( IsSequenceHaplotype( hapSeq), "The sequence is not haplotype." );

    // Now chek every seqeunce in matrix
    for(int r=0; r < GetRowNum(); ++r)
    {
        SEQUENCE row;
        GetRow( r, row );
        if( AreHapGenoRowsSame( hapSeq, row ) == true )
        {
            return true;
        }
        else if( AreHapGenoRowCompatible(hapSeq, row) == true )
        {
            return true;
        }
    }
    return false;
}

bool GenotypeMatrix :: CanBePhasedByHCSeqs( const set<int> &hcSeqs ) const
{
    // Check whether this geno mat can be phased by the set of hypercube seqs
    // Here, we simply form a genotype row by any of the two hypercube seqs
    set<int> genoRowIndices;
    vector<int> vecHCSeqs;
    PopulateVecBySet( vecHCSeqs, hcSeqs );
    for( unsigned  int i=0; i<vecHCSeqs.size(); ++i )
    {
        SEQUENCE s1;
        GetHyperCubeSeq( vecHCSeqs[i], s1, GetColNum() );
        for( unsigned int j=i; j<vecHCSeqs.size(); ++j)
        {
            // Now we see if the genotype formed by the two is what?
            SEQUENCE s2;
            GetHyperCubeSeq( vecHCSeqs[j], s2, GetColNum() );
            // Now try every genotype row
            for(int r = 0; r<GetRowNum(); ++r)
            {
                if( genoRowIndices.find( r ) != genoRowIndices.end() )
                {
                    continue;
                }
                SEQUENCE row;
                GetRow( r, row );
                if( CanPhaseGenoRow(s1, s2, row) == true)
                {
                    genoRowIndices.insert( r );
                }
            }
        }
    }
    if(genoRowIndices.size() == (unsigned int)GetRowNum())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GenotypeMatrix :: GetHapHCSeqs( set<int> &hcSeqs ) const
{
    hcSeqs.clear();
    // This function takes the set of haplotype rows in the genotype matrix
    for(int i=0; i<GetRowNum(); ++i)
    {
        SEQUENCE row;
        GetRow( i, row );
        if(IsSequenceHaplotype ( row ) == true )
        {
            // Yes, this is a haplotype row
            int nid = GetSeqIdFromSeq( row ) ;
            hcSeqs.insert( nid );        

        }
    }
}

bool GenotypeMatrix :: IsColComplement( int c1, int c2) 
{
	for(unsigned int i=0; i<rowsArray.size(); ++i)
	{
		if( rowsArray[i][c1] != 2 && rowsArray[i][c2] != 2)
        {
		    if( rowsArray[i][c1] == rowsArray[i][c2] )
		    {
			    return false;
		    }
        }
        else
        {
		    if( rowsArray[i][c1] != rowsArray[i][c2] )
		    {
			    return false;
		    }
        }
	}
	return true;
}

bool GenotypeMatrix :: IsColDuplicate( int c1, int c2) 
{
	for(unsigned int i=0; i<rowsArray.size(); ++i)
	{
		if( rowsArray[i][c1]  != rowsArray[i][c2]  )
		{
			return false;
		}
	}
	return true;
}


int GenotypeMatrix :: GetMajorityState( int site) 
{
    int res = 0;
    for(int r = 0; r<GetRowNum(); ++r)
    {
        if( GetValAt( r, site ) == 0 )
        {
            res += 2;
        }
        else if( GetValAt(r, site) == 2  )
        {
            res += 1;
        }
    }
    if( res >=  GetRowNum() )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
void GenotypeMatrix :: SetupCompanionColumns()
{
    // This function checks the data and fill in the companion rows for every pair of columns
    for(int i=0; i<nCols; ++i)
    {
        for(int j=i+1; j<nCols; ++j)
        {
            COLUMN_PAIR cp(i, j);
            set<int> cmpnRows;
            // The following 4 variables shows what are know already for (i,j)
            bool found00 = false, found01 = false;
            bool found10 = false, found11 = false;;
            for(int k=0; k<GetRowNum(); ++k)
            {
                if( rowsArray[k][i] == 2 && rowsArray[k][j] == 2 )
                {
                    cmpnRows.insert( k );
                }
                else if( rowsArray[k][i] == 0 && rowsArray[k][j] == 2 ) // Now check for forced pattern
                {
                    found00 = true;
                    found01 = true;
                }
                else if( rowsArray[k][i] == 1 && rowsArray[k][j] == 2 )
                {
                    found10 = true;
                    found11 = true;
                }
                else if( rowsArray[k][i] == 2 && rowsArray[k][j] == 0 )
                {
                    found00 = true;
                    found10 = true;
                }
                else if( rowsArray[k][i] == 2 && rowsArray[k][j] == 1 )
                {
                    found01 = true;
                    found11 = true;
                }
                else if( rowsArray[k][i] == 0 && rowsArray[k][j] == 0 )
                {
                    found00 = true;
                }
                else if( rowsArray[k][i] == 0 && rowsArray[k][j] == 1 )
                {
                    found01 = true;
                }
                else if( rowsArray[k][i] == 1 && rowsArray[k][j] == 0 )
                {
                    found10 = true;
                }
                else if( rowsArray[k][i] == 1 && rowsArray[k][j] == 1 )
                {
                    found11 = true;
                }

            }
            // Now we add this to our map
            if( cmpnRows.size() > 0)
            {
                companionRows.insert( COMPANION_ROW_MAP::value_type( cp, cmpnRows) );
            }

            // We also record the forced pattern
            if( found00 == true && found11 == true && found01 == true && found10 == true  )
            {
                // In this case we already have a complete pair
                completePairs.push_back( cp );
            }
            else
            {
                // This is not a complete pair
                if( found00 == true && found11 == true  )
                {
                    forcedColumnPairs.insert(FORCED_COL_MAP::value_type(cp, 0) );     // forced in phase
                }
                else if( found01 == true && found10 == true  )
                {
                    forcedColumnPairs.insert(FORCED_COL_MAP::value_type(cp, 1) );     // forced in phase
                }
            }

        }
    }
}

