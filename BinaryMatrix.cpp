#include "BinaryMatrix.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "Utils2.h"

// ***************************************************************************
// Define a reusable binary matrix class
// ***************************************************************************

BinaryMatrix :: BinaryMatrix() 
{
    nCols = 0;
}

BinaryMatrix :: ~BinaryMatrix()
{
	// Need to free up data if needed
	Clear(); 
}			   

BinaryMatrix :: BinaryMatrix(int nr, int nc)
{
	SetSize( nr, nc ); 
}


BinaryMatrix :: BinaryMatrix( const BinaryMatrix &rhs)
{
	Copy(rhs); 
}


BinaryMatrix & BinaryMatrix :: operator=(const BinaryMatrix &rhs)
{
	Clear(); 

	Copy(rhs); 

	return *this; 
}

			   

bool BinaryMatrix :: IsDataValid(int val) 
{
    if( val == 0 || val == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//#if 0
void BinaryMatrix :: TrimNgbrDupCompSites(set<int> *pRemovedSet )
{
	set<int> setOfRemovals;		// contains sites to be removed
	int cleft = 0;
	while( cleft < nCols-1)
	{
		// Check to see if the next row  immediately is complement or not 
		if( IsColComplement( cleft, cleft+1 ) == true || IsColDuplicate( cleft, cleft+1)== true)
		{
			setOfRemovals.insert( cleft+1 );
//cout << "Site " << cleft+1 << " is same/complement." << endl;
		}
		// Consider  next site
		cleft ++;
	}
    if( pRemovedSet != NULL)
    {
        pRemovedSet->clear();
        *pRemovedSet = setOfRemovals;
    }
	// Finally, remove columns
	RemoveColumns( setOfRemovals );
}
//#endif

// Consolidate columns in matrix
void BinaryMatrix:: TrimDupSites(set<int>* pRemovedSites )
{
	int c1, c2;
    unsigned int r;
	set<int> setOfDuplicates;		// contains sites to be removed

	for(c1 = 0; c1 < nCols; ++c1)
	{
		for(c2 = c1+1; c2 < nCols; ++c2)
		{
			// now we compare these two cols: c1, c2
			// if they match, we put c2 into set
			bool f = false;
			for(r = 0; r< rowsArray.size(); ++r)
			{
				// compare each cell
				if( rowsArray[r][c1] != rowsArray[r][c2]) 
				{
					f = true;
					break;
				}
			}

			// Check against size
			if(r == rowsArray.size() )
			{
				// we find a duplicate
				if( setOfDuplicates.find(c2) == setOfDuplicates.end() )
				{
//					cout <<  "Site " << c2 << " is duplicate of site ";
//					cout << c1 << endl;
				}
				setOfDuplicates.insert(c2);
		
            }

		}
	}
    // Now save the trimed sites info, if needed
    if(  pRemovedSites != NULL )
    {
        *pRemovedSites = setOfDuplicates;
    }	

	// Finally, remove columns
	RemoveColumns( setOfDuplicates );

}

int BinaryMatrix :: FindDupRow( )
{
	// This function tracking any removal of rows, but
	// in addition to it, we track which rows remains
	unsigned int r1, r2;
    int c;

	for(r1=0; r1<rowsArray.size(); ++r1)
	{
		for( r2 = r1+1; r2 < rowsArray.size(); ++r2)
		{
			/*
				Now test whether row 1 and row 2 are the same
			*/
			bool fSame = true;
			for(c = 0; c< nCols; ++c)
			{
				if(rowsArray[r1][c] != rowsArray[r2][c])
				{
					fSame = false;
					break;
				}
			}
			if(  fSame )
			{
//cout << "row " << r2 << " is duplicate." << endl;
				return  r2 ;
			}
		}
	}

	return -1;
}


/*
	Remove all non-informative sites
	A site is non-informative if it is all 0 (1), or has only single 0(1)
*/
bool BinaryMatrix :: TrimNonInformativeSites( set<int> *pRemovedSet  )
{
	int c1;
    unsigned int r;
	set<int> setOfDuplicates;		// contains sites to be removed

	for(c1 = 0; c1 < nCols; ++c1)
	{
		int numZeros = 0, numOnes = 0;
		// now we compare these two cols: c1, c2
		// if they match, we put c2 into set
		for(r = 0; r< rowsArray.size(); ++r)
		{	
			if(rowsArray[r][c1] == 0)
			{
				numZeros ++;
#if 0
				if(numZeros >=2 && numOnes >=2)
				{
					break;
				}
#endif
			}
			else if(rowsArray[r][c1] == 1)
			{
				numOnes ++;
#if 0
				if(numZeros >=2 &&  numOnes >= 2)
				{
					break;
				}
#endif
			}
		}
		// Check to see if this is non-informative
		if( numZeros <=1 || numOnes <=1  )
		{
			// we find a duplicate
//			cout << "Site  " << c1+1 << "is  non-informative" << endl;
			setOfDuplicates.insert(c1);
		}	
	}
    if( pRemovedSet != NULL)
    {
        pRemovedSet->clear();
        *pRemovedSet = setOfDuplicates;
    }

	// Finally, remove columns
	bool res = false;
	if( setOfDuplicates.size() > 0)
	{
		res = true;
		RemoveColumns( setOfDuplicates );
	}
	return res;
}

void BinaryMatrix :: TrimFullyCompatibleSites( set<int> *pRemovedSet )
{
	int c1, c2; 
	set<int> setOfDuplicates;		// contains sites to be removed
	for(c1 = 0; c1 < nCols; ++c1)
	{
		// now we compare these two cols: c1, c2
		// if they match, we put c2 into set
		bool f = true;   // by default, we say f is fully-compatible		
		// Now we test whether sites c1 is compatible with c2
		for(c2 = 0; c2 < nCols; ++c2)
		{
			if( IsCompatible( c1, c2 ) == false)
			{
				f = false;
				break;
			}
		}
		if( f == true && IsColumnBinary( c1 ) == true)
		{
//cout << "Site " << c1+1 << " is fully compatible" << endl;
			setOfDuplicates.insert(c1);
		}
	}

    // Now remember the set if needed
    if( pRemovedSet != NULL)
    {
        pRemovedSet->clear();
        *pRemovedSet = setOfDuplicates;
    }

	// Finally, remove columns
	RemoveColumns( setOfDuplicates );
}


bool BinaryMatrix :: IsAllColumnsUnique()
{
	bool res = true; 

	for(int i= 0; i<nCols-1; ++i)
	{
		for(int j=i+1; j<nCols; ++j)
		{
			// check to see if column i, j are duplicate
			if ( CmpColumns ( i, j) == true )
			{
				return false; 
			}
		}
	}


	return res;
}

bool BinaryMatrix :: IsColNonInformative(int c, int *singletonState )
{
	int numZeros = 0, numOnes = 0;
	// now we compare these two cols: c1, c2
	// if they match, we put c2 into set
	for(unsigned int r = 0; r< rowsArray.size(); ++r)
	{	
		if(rowsArray[r][c] == 0)
		{
			numZeros ++;
#if 0
			if(numZeros >=2 && numOnes >=2)
			{
				break;
			}
#endif
		}
		else if(rowsArray[r][c] == 1)
		{
			numOnes ++;
#if 0
			if(numZeros >=2 &&  numOnes >= 2)
			{
				break;
			}
#endif
		}
	}
	// Check to see if this is non-informative
	if( numZeros ==1 || numOnes ==1  )
	{
        if( singletonState != NULL)
        {
            if( numZeros == 1 )
            {
                *singletonState = 0;
            }
            else
            {
                *singletonState = 1;
            }
        }
		// we find a duplicate
//			cout << "Site  " << c1+1 << "is  non-informative" << endl;
        return true;
	}	
    else
    {
        return false;
    }
}


bool BinaryMatrix :: IsColTrivial (int c)
{
	// check whether column c is trivial or not
	// a column is trivial if the column is all 0 or all 1
	bool hasZero = false;
	bool hasOne = false;
	for(int i=0; i<GetRowNum(); ++i)
	{
		if( rowsArray[i][c]   == 0)
		{
			hasZero = true;
		}
		else
		{
			hasOne = true;
		}
	}
	if( hasZero && hasOne)
	{
		return false; 
	}
	else
	{
		return true; 
	}
}

void BinaryMatrix :: GetTrivialSites( vector<int> &trivSites )
{
    trivSites.clear();
    for( int c=0; c< GetColNum(); ++c)
    {
        if( IsColTrivial(c) == true )
        {
            trivSites.push_back( c );
        }
    }
}


bool  BinaryMatrix ::  IsSequencesMatch( int r1, int r2, vector<int>& seqColPos  )  
{
	bool res = true;
//cout << "r1 = " << r1 << ", r2 = " << r2 << ", seqeucne lpocation are ";
//DumpIntVec( seqColPos ); 

	// This function test whether (non-continuous) sequences for two rows match or not
	for(unsigned int i=0; i<seqColPos.size(); ++i)
	{
		if( rowsArray[r1][ seqColPos[i] ] != rowsArray[r2][ seqColPos[i] ] )
		{
			res = false;
			break;
		}
	}
	return res;
}

bool BinaryMatrix :: IsZeroColumn( int c)
{
	bool res = true;
	for(unsigned int i=0; i<rowsArray.size(); ++i)
	{
		if(rowsArray[i][c] == 1)
		{
			res = false;
			break;
		}
	}
	return res;
}

int  BinaryMatrix :: GetZeroColNum()
{
	int res = 0;
	for(int i=0; i<nCols; ++i)
	{
		if(IsZeroColumn ( i ) )
		{
			res ++; 
		}
	}
	return res; 
}

 
void BinaryMatrix :: BuildColEquivClasses()
{
	for(int i=0; i<nCols; ++i)
	{
		bool f = false;
		for( COLUMN_EQUIV_CLASS::iterator it = setColEquiv.begin(); it != setColEquiv.end(); ++it  )
		{
			set<int> &s = *it; 

			// check to see if column i/j are the same
			if( CmpColumns (i, *(s.begin())    ) == true  )
			{
				// remember this fact in the map
				f = true; 
				s.insert(i); 
				break; 
			}
		}

		if( f == false)
		{
			// Create a new set
			set<int> s1; 
			s1.insert(i); 
			setColEquiv.push_back( s1 ); 
		}

	}
}

 
void BinaryMatrix :: GetUniqueColsInRange( int c1, int c2, set<int> &setUniques  ) 
{
	// make sure equiv class is pre-processed
	if (setColEquiv.empty() )
	{
		BuildColEquivClasses(); 
	}

	// exam each column equivlance classes
	// Put the mostly diesired (for now, it is the one near the center)
	// into result set (which must be in range)
	int center = (c1+c2)/2; 
	for(unsigned int i=0; i<setColEquiv.size(); ++i)
	{
		set<int> &s = setColEquiv [i]; 
		int cand = -100; 
		for(set<int> ::iterator it = s.begin(); it != s.end(); ++it)
		{
			int c = *it; 
			if( c >=c1 && c<= c2 && abs( cand - center) > abs(c - center) )
			{
				cand = c; 
			}
		}
		if( cand >= 0)
		{
	 		setUniques.insert ( cand  );
		} 
	
	}

}

bool BinaryMatrix :: IsPerfectPhylogeny()
{
	for(int i=0; i<nCols-1; ++i)
	{
		for(int j=i+1; j<nCols; ++j)
		{
			if(IsCompatible (i, j) == false)
			{
//cout << "Site i=" << i << ", j=" << j << " are incompatible.\n";
				return false; 
			}
		}
	}
	return true; 
}


void BinaryMatrix  ::  ConstructConflictGraph( UnWeightedGraph &graph )
{
	// Conflict graph vertex num = # of columns
	// Edge is whether col i conflict with col j
	LIST_VERTEX vertList;
	LIST_EDGE edgeList ;

	for(int i=0; i<nCols; ++i)
	{
		char buf[100];
		buf[0] = 'c'; 
		sprintf( &buf[1], "%d", i+1 );
		BGVertex v( buf ); 
		vertList.push_back ( v ); 
	}
	graph.SetVertices ( vertList); 

	// Now check for all pair of columns for conflict
	for(int i=0; i<nCols-1; ++i)
	{
		for(int j=i+1; j<nCols; ++j)
		{
			if(IsCompatible (i, j) == false) 
			{
//cout << "Add one edge (" << i << " , " << j << ")" << endl; 
				BGEdge eg("e", i, j, graph.GetListVerts()   ) ;
				edgeList.push_back( eg );
			}
		}
	}


	// Finally, setup the vertex\edge lists
	graph.SetEdges (edgeList);

}




bool BinaryMatrix :: IsColumnBinary(int c) const
{
	for(int i=0; i<GetRowNum(); ++i)
	{
		if(rowsArray[i][c] !=0 && rowsArray[i][c] != 1)
		{
			return false;
		}
	}
	return true;
}

bool BinaryMatrix :: IsRowBinary(int r) const
{
	for(int i=0; i<nCols; ++i)
	{
		if( rowsArray[r][i] != 0 && rowsArray[r][i] != 1)
		{
			return false;
		}
	}
	return true;
}
 
void BinaryMatrix :: TrimNonBinaryRows()
{
	set<int> setOfDuplicates;
	setOfDuplicates.clear(); 
	unsigned int r1;
    //int c;

	for(r1=0; r1<rowsArray.size(); ++r1)
	{
		if( IsRowBinary(r1) == false)
		{
			// The row with duplicated rows are treated the same
		//cout << "row " << r2 << " is not binary." << endl;
			setOfDuplicates.insert(  r1 );
		}
	}
	/*
		Now we remove all duplicate rows 
	*/
	RemoveRows( setOfDuplicates ); 

}

bool BinaryMatrix :: IsRowRangeBinary( int r, int left, int right  )
{
	for(int i=left; i<=right; ++i)
	{
		if( rowsArray[r][i] == 2)
		{
			return false;
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
//		Inernal utility functions
///////////////////////////////////////////////////////////////////////////////////////////


bool BinaryMatrix :: IsCompatible( int c1, int c2)
{
	bool f00 = false;
	bool f01 = false;
	bool f10 = false;
	bool f11 = false;

	// if c1==c2, we assume it is compatible
	if(c1 == c2)
	{
		return true;
	}
#if 0       // no, acutally, we need to be more cautious, unless we see evidence, we put it
	// For now, if a column is not binary, we consider it is not compatible
	if( IsColumnBinary(c1) == false || IsColumnBinary(c2) == false)
	{
		return false;
	}
#endif
	// 4-gamet test
	for(unsigned int i=0; i<rowsArray.size(); ++i)
	{
		if(rowsArray[i][c1] == 0 && rowsArray[i][c2] == 0)
		{
			f00 = true;
		}
		if(rowsArray[i][c1] == 0 && rowsArray[i][c2] == 1)
		{
			f01 = true;
		}
		if(rowsArray[i][c1] == 1 && rowsArray[i][c2] == 0)
		{
			f10 = true;
		}
		if(rowsArray[i][c1] == 1 && rowsArray[i][c2] == 1)
		{
			f11 = true;
		}
	}

	// Now check to see if all flags are set
	if(f00 && f01 && f10 && f11)
		return false;
	else
		return true;
}

bool BinaryMatrix :: IsColComplement( int c1, int c2)
{
	for(unsigned int i=0; i<rowsArray.size(); ++i)
	{
		if( rowsArray[i][c1] == rowsArray[i][c2] || rowsArray[i][c1] == 2 || rowsArray[i][c2] == 2)
		{
			return false;
		}
	}
	return true;
}
bool BinaryMatrix :: IsColDuplicate( int c1, int c2)
{
	for(unsigned int i=0; i<rowsArray.size(); ++i)
	{
		if( rowsArray[i][c1]  != rowsArray[i][c2] ||  rowsArray[i][c1] == 2 || rowsArray[i][c2] == 2)
		{
			return false;
		}
	}
	return true;
}

void BinaryMatrix :: GetAllIncompatiblePairs(set< pair<int,int> > &incompatibles)
{
    incompatibles.clear();
	for(int i=0; i<nCols ; i++)
	{
		for(int j= i+1 ; j<nCols; ++j)
		{
			// Test to see if site i, j are compatible
			if(  IsCompatible(i, j) == false)
			{
                pair<int,int> p(i,j);
                incompatibles.insert(  p    ); 
            }
		}
	}
}

int BinaryMatrix :: ComputeHKBound()
{
	// The idea is to test for incompatible between each column
	// Then create an incompatibility map, and compute the bound
	map<INTERVAL, int> bounds;

	int nCols = GetColNum();
	int nRows = GetRowNum();
	if( nCols <=1 || nRows <=3)
	{
		return 0;
	}

	for(int i=0; i<nCols-1; ++i)
	{
		for(int j=i+1; j<nCols; ++j)
		{
			// Check if site i, j conflict
			int val = 0;
			if( IsCompatible(i, j) == false)
			{
				val = 1;
			}
			INTERVAL iv(i, j);
			bounds.insert( map<INTERVAL,int>::value_type(iv, val) );
		}
	}
    vector<int> locBreakpoints;     // do not really need this, but...
	return CalcCompositeBound(bounds, 0, nCols-1, locBreakpoints);
}

int BinaryMatrix :: ComputeFastHapBound()
{
    // Simply test for each submatrix for a rough haplotype bound
	// Then create an incompatibility map, and compute the bound
    // To speed things up, we do not perform optimal RecMin
    // Rather simply no-subset

	map<INTERVAL, int> bounds;

	int nc = GetColNum();
	int nr = GetRowNum();
	if( nc <=1 || nr <=3)
	{
		return 0;
	}

	for(int i=0; i<nc-1; ++i)
	{
		for(int j=i+1; j<nc; ++j)
		{
			// Check if site i, j conflict
			int val = 0;

            BinaryMatrix submat;
	        SubMatrix ( 0, GetRowNum() - 1, i, j, submat); 
	        submat.TrimFullyCompatibleSites(); 
	        submat.TrimDupRows(); 

            val = submat.GetRowNum() - submat.GetColNum() - 1;
            if( val < 0 )
            {
                val = 0;
            }

			INTERVAL iv(i, j);
			bounds.insert( map<INTERVAL,int>::value_type(iv, val) );
//cout << "interval " << i << ", " << j  << " quick bd = " << val << endl; 
		}
	}
    vector<int> locBreakpoints;     // do not really need this, but...
	return CalcCompositeBound(bounds, 0, nc-1, locBreakpoints);
}

// This function computes a fast recombination upper bound, which can be useful in applications like branch and bound
// The idea is to remove a sequence from inputmat a time, and take the min to recombine them
int BinaryMatrix :: ComputeFastRecombUpperBound()
{
    // Create a new sequence for operation
    BinaryMatrix matToOp = *this;

    int res = 0;
    // Whenver the matrix is too small, we stop
    while( true )
    {
        // First perform cleanup: drop non-informatives rows, collapse identical rows
        set<int> setOfRemoved;
  	    matToOp.TrimFullyCompatibleSites( &setOfRemoved );
	    matToOp.FindNgbrDupCompSites( &setOfRemoved );
        matToOp.RemoveColumns( setOfRemoved );
        matToOp.TrimDupRows();

        if ( matToOp.GetRowNum() <= 3 )
        {
            break;
        }

        // Find the smallest cost row
        int minRmCost = HAP_MAX_INT;
        int minRow = -1;
        // Try every leftover row in matToOp
        for(int r = 0; r < matToOp.GetRowNum(); ++r)
        {
//            SEQUENCE row;
//            matToOp.GetRow( r, row );
            int recCost = matToOp.ComputeMinRecombWeight( r );
            if( recCost < minRmCost )
            {
                minRmCost = recCost;
                minRow = r;
            }
        }
        YW_ASSERT_INFO( minRow >= 0, "Error: minRow must be updated at least once." );
//cout << "minRmCost = " << minRmCost << ", minRow = " << minRow << endl;
        // Now we remove this sequence
        res += minRmCost;
        set<int> seqsToRemove;
        seqsToRemove.insert( minRow );
        matToOp.RemoveRows( seqsToRemove );
    }
//cout << "A fast recomb. upper bound = " << res << endl;
    return res;
}

int BinaryMatrix :: ComputeMinRecombWeight( int rowIndex )
{
    // This function computes a recombination number given the rows in matrix 
    // that are ancesters of rowIndex
    // This function computes the minimum recombination weight for the given hapRow
    // when restricted to interval [left, right] in mat
    int res = 0;
//cout << "ComputeMinRecombWeight :: rowIndex = " << rowIndex << endl;
//cout <<"matrix here is: ";
//Dump();
    set<int> lastTrackRows;  // set of rows that matching the hapRow

    // Initially every row is a match
    for(int i=0; i<GetRowNum(); ++i)
    {
        if( i != rowIndex )
        {
            lastTrackRows.insert( i );
        }
    }


    for(int curpos=0; curpos < GetColNum(); ++curpos)
    {
        // Each time, we intersect the set with the sets matching the current bit
        set<int> trackRows;
        for(int i=0; i< GetRowNum(); ++i)
        {
            if( i == rowIndex)
            {
                continue;
            }

            if( GetValAt(i, curpos) == GetValAt(rowIndex, curpos) )
            {
                // Yes, this row matches
                trackRows.insert( i );
            }
        }
        YW_ASSERT_INFO( trackRows.size() > 0, "trackRows must contain some rows." );

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

//cout << "Min recomb = " << res << endl;
    return res;
}

int BinaryMatrix :: GetMajorityState( int site) 
{
    int res = 0;
    for(int r = 0; r<GetRowNum(); ++r)
    {
        if( GetValAt( r, site ) == 0 )
        {
            res ++;
        }
    }
    if( res >= (GetRowNum()+1)/2 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int BinaryMatrix :: GetMinorStateNum( int site, int &minorState )  const
{
    int res = 0;
    for(int r = 0; r<GetRowNum(); ++r)
    {
        if( GetValAt( r, site ) == 0 )
        {
            res ++;
        }
    }
    if( res >= (GetRowNum()+1)/2 )
    {
        minorState = 1;
        return GetRowNum() - res;
    }
    else
    {
        minorState = 0;
        return res;
    }
}

