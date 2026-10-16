#include "BlockMCMC.h"
#include <cmath>
#include "HeuristicBlock.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// important data structure

BlockMCMCMain :: BlockMCMCMain( const BinaryMatrix &mat, int nF ) : inputMat( mat ), numFounders( nF)
{
#ifdef SIMPLE_INIT
    Init();
#else
    InitHeurstic();
#endif
}

BlockMCMCMain :: ~BlockMCMCMain()
{
}



int BlockMCMCMain :: Run( int steps )
{
    double  bestScore = this->curScore;
    // Now we run this many steps
    for( int s = 0; s< steps; ++s)
    {
        // with 1/2 chance, we do nothing
        double rndVal = GetRandFraction();
        if( rndVal < 0.5 )
        {
            continue;
        }

        // Now we perform one update
        Update();

        // Found a better score?
        if( this->curScore < bestScore )
        {
            bestScore = (int)this->curScore;
        }
    }
cout << "Final state = " << this->curScore << endl;
    // return the current score
    return  (int)bestScore;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Run MCMC for certain steps
void BlockMCMCMain :: Init()
{
    // Here we simply assign a trivial solution
    // That is, assume founder 1 are all-0 and founder 2 are all-1, and nothing else
    for(int r = 0; r < inputMat.GetRowNum( ); ++r)
    {
        vector<int> rowFounderStates;
        for(int c=0; c<inputMat.GetColNum(); ++c)
        {
            // is this bit zero?
            if( inputMat(r,c) == 0 )
            {
                rowFounderStates.push_back( 0 );
            }
            else
            {
                rowFounderStates.push_back( 1 );
            }
        }
        // Now add this row
        curFounderStates.push_back( rowFounderStates );
    }
    // also update the score
    this->curScore =  CalcScore( curFounderStates );
}



void BlockMCMCMain :: InitHeurstic()
{
    // We start by running with a heuristic version to get a solution near optimal
    GreedyRecBlockParser recHeuristicSolver( inputMat, this->numFounders  );
    recHeuristicSolver.Solve();
    FOUNDER_LIST foundersNO;
    recHeuristicSolver.GetSolFounders( foundersNO );
    
    // Now we set up row by row
    for(int r = 0; r < inputMat.GetRowNum( ); ++r)
    {
        vector<int> rowFounderStates;
        SEQUENCE seq;
        inputMat.GetRow( r, seq );

        GetFounderState( foundersNO, seq,  rowFounderStates);
//cout << "processing row = ";
//DumpSequence( seq );
//cout << "Founder state = ";
//DumpSequence( rowFounderStates );
        // Now add this row
        curFounderStates.push_back( rowFounderStates );
    }

    // also update the score
    this->curScore =  CalcScore( curFounderStates );
}


void BlockMCMCMain :: Update()
{
//cout << "On entry of update: ";
//Dump();

    // This function implements a simple local update function. 
    // It pick a random site, and then pick any cell in that cell, and replace the founder state with an available state
    // A state is availabe if it is not being current used at all or is being used only for cell with same value (0/1)

    int rndSite = (int) ( inputMat.GetColNum() * GetRandFraction() );

    // pick a random cell too
    int rndRow = (int) ( inputMat.GetRowNum() * GetRandFraction() );
//cout << "rand site = " << rndSite << ", rnd row = " << rndRow << endl;

    set<int> conflictFounders;
    for(  int i = 0; i< inputMat.GetRowNum(); ++i  )
    {
        // Is this bit same as that or rndRow
        if( inputMat(i, rndSite) != inputMat(rndRow, rndSite)  )
        {
            conflictFounders.insert( curFounderStates[i][rndSite]  );
        }
    }

    vector<int> availFounders;
    for(int f = 0; f< numFounders; ++f)
    {
        if( conflictFounders.find( f ) == conflictFounders.end()  )
        {
            availFounders.push_back( f );
        }
    }
//cout << "avialFounders = ";
//DumpIntVec( availFounders );
    // In case we can not move forward, skip this step
    //if( availFounders.size() == 0 )
    //{
    //    return;
    //}

    YW_ASSERT_INFO( availFounders.size() > 0, "Something very wrong." );

    // Now randomly pick a founder from the list and use it to replace all cell with that value there
    int rndFounderIndex = (int) ( availFounders.size() * GetRandFraction() );
    int founderNew = availFounders[rndFounderIndex];
//cout << "Pick a random founder = " << founderNew << endl;

    // THis is the candidate MCMC states
    vector< vector<int> > stateUpdate = this->curFounderStates;
    for( int r=0; r<inputMat.GetRowNum(); ++r)
    {
        if( stateUpdate[r][rndSite]  == curFounderStates[rndRow][rndSite]  )
        {
            stateUpdate[r][rndSite] = founderNew;
        }
    }
    
    // Test whether we should acdept it or not by Metropolis filter
    double scoreNew = CalcScore( stateUpdate );
    double relScore = CalcRelativeScore( scoreNew, curScore );
//cout << "scoreNew = " << scoreNew << ", curSocre = " << curScore  <<  ", relscore = " << relScore << endl;
    
    double rndval = GetRandFraction();
    if( rndval < relScore )
    {
        // accept this change
        curFounderStates = stateUpdate;
        curScore = scoreNew;
//cout << "Update to new score = " << curScore << endl;
//Dump();
    }
    else
    {
        // do nothing
    }
}

double BlockMCMCMain :: CalcScore( const BLOCK_MCMC_STATE &state)
{
    // We simply count the number of crossovers
    double res = 0.0;

    for( unsigned int r = 0; r< state.size(); ++r )
    {
        for( unsigned int c = 0; c < state[r].size() -1; ++c)
        {
            if( state[r][c] != state[r][c+1]  )
            {
                res += 1.0;
            }
        }
    }
    YW_ASSERT_INFO( res >= 0.5, "Something wrong" );
    return res;
}

double BlockMCMCMain :: CalcRelativeScore( double scoreNew, double scoreOld)
{
    // We simply use the 2^(sOld - sNew)
    return pow (10.0,  scoreOld - scoreNew - 1 );
}

void BlockMCMCMain :: Dump()
{
    cout << "Current MCMC state is : \n";
    for( unsigned int r=0; r<curFounderStates.size(); ++r  )
    {
        DumpSequence( curFounderStates[r] );
    }
}
