#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
using namespace std;

#include "Utils.h"
#include "Utils2.h"
#include "BinaryMatrix.h"
#include "BlockUtils.h"
#include "HeuristicBlock.h"
#include "BlockMCMC.h"
#include "ExactBlock.h"

// Some global paramemters
static int fileArgIndex = 1;
static int numFounders = 3;
static int upperBound = -1;
static int huerisExeCount = 1000;
static bool fGenotype = false;
static bool fApproxOnly = false;


// Useful local functions
static void Usage()
{
    cout << "Usage: RecBlcok -<options> <file-name>";
    cout << "Options:\n";
    cout << "   F<k>: set number of founders to be k\n";
    cout << "   D<k>: set approximation depth to k = 0\n"; 
    cout << "         (fastest but may give less optimal result), 1, 2, and so on\n";
    cout << "   C<k>: limit number of search pathes to k (say 100, 1000, etc)\n";
    cout <<"For example: type './RecBlock -F4 tmp.dat' to compute the EXACT number of breakpoints\n";
    cout << "in a minimum mosaic structure assuming four founders.\n";
    cout <<"Type './RecBlock -F4 -C500 -D1 tmp.dat' to compute approximate number of breakpoints\n";
    cout << "in a minimum mosaic structure.\n";
    exit(1);
}

static bool CheckArguments(int argc, char **argv) 
{
    if( argc == 1 )
    {
        return false;
    }


    // Check argument one by one
    for(int i = 1; i< argc; ++i)
    {
		if(strncmp ( argv[i], "-F", 2) == 0)
        {
            // Then it means to input number of recombs
            sscanf(argv[i]+2, "%d", &numFounders ); 
cout << "Set founder number to " << numFounders << endl;
        }

		else if(strncmp ( argv[i], "-U", 2) == 0)
        {
            sscanf(argv[i]+2, "%d", &upperBound ); 
cout << "Set upper bound to " << upperBound << endl;
        }
		else if(strncmp ( argv[i], "-H", 2) == 0)
        {
            sscanf(argv[i]+2, "%d", &huerisExeCount ); 
cout << "Set heuristic executation count to " << huerisExeCount << endl;
        }
		else if(strncmp ( argv[i], "-G", 2) == 0)
        {
            fGenotype = true;
cout << "Set to genotype data\n";
        }
		else if(strncmp ( argv[i], "-A", 2) == 0)
        {
            fApproxOnly = true;
cout << "Only run the approximate quick method\n";
        }

		else if(strncmp ( argv[i], "-D", 2) == 0)
        {
            int nDepth;
            sscanf(argv[i]+2, "%d", &nDepth ); 
            SetApproxLimitControl( nDepth );
cout << "Set fast founder depth to " << nDepth << endl;
            fApproxOnly = true;
        }
		else if(strncmp ( argv[i], "-C", 2) == 0)
        {
            int nConfig;
            sscanf(argv[i]+2, "%d", &nConfig ); 
            SetMaxConfigNumStep( nConfig );
cout << "Set maximum number of kept configurations to " << nConfig << endl;
            fApproxOnly = true;
cout << "Only run the approximate quick method\n";
        }
        else if( argv[i][0] != '-' )
        {
            // not an option one. Right now the only one is file
            fileArgIndex = i;
        }
        else
        {
            return false;
        }
    }


    return true;
}

static void GenoMnBlockFunc(const GenotypeMatrix &inputGenoMat, int numFounders )
{
//cout << "inputGenoData = ";
//inputGenoMat.Dump();

// be careful: config is changed to add something called isomorphic settings
//YW_ASSERT_INFO(false, "Sorry, the genotype version is currently under development.");

    // compute a lower bound first
    long tstart1 = GetCurrentTimeTick();
    ExactEnumGenoFoundersSolver exactESolver( inputGenoMat, numFounders  );
    exactESolver.SetDispFlag( true );

    // set approx flag if needed
    if(fApproxOnly == true)
    {
        exactESolver.SetApproxFlag( true );
    }


    int res = exactESolver.Solve();
//    int res = exactESolver.SolveDFS();
    cout << "Min number of breakpoints for this genotype data is " << res << endl;
    FOUNDER_LIST founderExact;
//    exactESolver.GetSolFounders(founderExact);
    //exactESolver.GetDFSSolFounders(founderExact);

    // dump out
    BinaryMatrix phasedMat;
    exactESolver.GetPhasedMatrix( phasedMat, founderExact );
    cout << "Exact founders are: \n";
    DumpVecSequences( founderExact );
    cout << "Phased matrix = ";
    phasedMat.Dump();
    cout << "Founder parse are: \n";
    DumpParseMatrixByFounders( founderExact, phasedMat);

    
    
    cout << "Elapsed time = " << GetElapseTime( tstart1 ) << " seconds." << endl;

}



int main( int argc, char **argv  )
{
    if( CheckArguments( argc, argv) == false)
    {
        Usage();
    }

	ifstream inFile(argv[fileArgIndex]);
	if(!inFile)
	{
		cout << "Can not open "<< argv[fileArgIndex] <<endl;
		exit(1);
	}

    // is this genotype mode?
    if( fGenotype == true )
    {
        GenotypeMatrix inputGenoMat;
        inputGenoMat.ReadFromFile(inFile);
        inFile.close();
        GenoMnBlockFunc(inputGenoMat, numFounders);
        exit(0);
    }


    BinaryMatrix inputMat;
    inputMat.ReadFromFile (inFile );
    inFile.close();

    // Remove ngbr duplidate sites and non-informative sites here
    // what about fully compatible sites, I do not know. TBD
//#if 0
    set<int> ngbrDupSites;
    inputMat.FindNgbrDupCompSites( &ngbrDupSites );
    inputMat.RemoveColumns( ngbrDupSites );
    // also trim the trivial sites too
    vector<int> trivSites;
    inputMat.GetTrivialSites(trivSites);
    set<int> nonInformSitesMat;
    PopulateSetByVec(nonInformSitesMat, trivSites );
    inputMat.RemoveColumns( nonInformSitesMat );
    //set<int> setOfRemoved;
	//inputMat.TrimFullyCompatibleSites( &setOfRemoved );
    // now trim dup rows (should we do this?)
//    inputMat.TrimDupRows();
//#endif
cout <<"After preprocessing, matrix = ";
inputMat.Dump();

    if( inputMat.GetColNum() <= 1 )
    {
        cout << "No work to do. The matrix is too small.\n";
        exit(0);
    }

#if 0
    BlockILPSolver  blockILP( inputMat, numFounders );
    blockILP.OutputILP4( "tmp4.ILP" );
cout << "tmp4.ILP outputed.\n";

    blockILP.OutputILP( "tmp.ILP" );
cout << "tmp.ILP outputed.\n";
    blockILP.OutputILP2("tmp2.ILP");
cout << "tmp2.ILP outputed.\n";
    blockILP.OutputILP3("tmp3.ILP");
cout << "tmp3.ILP outputed.\n";
    blockILP.OutputILPLowerBound("tmpLB.ILP");
cout << "tmpLB.ILP outputed.\n";
#endif
    // Now computes a lower bound on it
    int lbRec = ComputeBlockLB( inputMat, numFounders );
    cout << "The minimum number of crossovers is at least " << lbRec << endl;


    // this takes history into consideration
//    int lbRecHist = ComputeBlockHistoryLB( inputMat, numFounders );
//    cout << "The minimum number of history crossovers is at least " << lbRecHist << endl;

    // Now call founder check
//    ExamWndUniqueness( 6, numFounders );
//exit(1);

    // Now check the performance of heuristic way of building recombination blocks

//    int cntOnetime = blockApproxSolver.SolveOneCall();
//    cout << "A simple heuristic call gives " << cntOnetime << endl;


    // timestamp it
    long tstart1 = GetCurrentTimeTick();



//#if 0
    // solution found by quick search
    FOUNDER_LIST founderFast;

    int cnt = upperBound;
    if(  upperBound < 0 )
    {
#if 0
        GreedyRecBlockParser blockApproxSolver(inputMat, numFounders);
        cnt = blockApproxSolver.Solve( huerisExeCount );
        cout << "Heuristic solver solution = " << cnt << endl;

        FOUNDER_LIST heuristicFounders;
        blockApproxSolver.GetSolFounders(heuristicFounders);

        // dump out
        cout << "Heuristic founders are: \n";
        DumpVecSequences( heuristicFounders );
        cout << "Founder parse are: \n";
        DumpParseMatrixByFounders( heuristicFounders, inputMat);
#endif


cout << "\n\nFirst run a fast but potoentially NON-optimal search...\n";
        // try fast version of program
        ExactEnumFoundersSolver fastESolver( inputMat, numFounders, HAP_MAX_INT );
#if 0
cnt = fastESolver.SolveTestCode();
fastESolver.GetSolFounders(founderFast);
cout << "TEST CODE ONLY: founders are: (NOT NECCESSARILY OPTIMAL) \n";
DumpVecSequences( founderFast );
cout << "Founder parse are: \n";
DumpParseMatrixByFounders( founderFast, inputMat);
#endif


        //fastESolver.SetDispFlag(false);
        fastESolver.SetApproxFlag( true );
        cnt = fastESolver.Solve();

        fastESolver.GetSolFounders(founderFast);

        // dump out
        cout << "APPROXIMATE founders are: (NOT NECCESSARILY OPTIMAL) \n";
        DumpVecSequences( founderFast );
        cout << "Founder parse are: \n";
        DumpParseMatrixByFounders( founderFast, inputMat);

    }

    // if that is the only thing we need, stop here
    if( fApproxOnly == true )
    {
        // how long does it take?
        cout << "Elapsed time = " << GetElapseTime( tstart1 ) << " seconds." << endl;

        exit (0 );
    }


//#endif
    // Now we compute a min number by brute force
//    int optRes = CalcMinCrossoverBruteForce( inputMat, numFounders);
//    cout << "Optimal solver solution by brute force  = " << optRes  << endl;

    // Now we try MCMC here
#if 0
    BlockMCMCMain mcmcRunner( inputMat, numFounders );
    int numSteps = 1000000;
    int score = mcmcRunner.Run( numSteps );
    cout << "After a number of steps, the score = " << score << endl;

    ExactBlockSolver exactBSolver( inputMat, numFounders, cnt );
    exactBSolver.Solve();
cout << "We are done.\n";
#endif

cout <<"\n\nNow we run the FULL search process...\n";

    //cnt = 90;
    ExactEnumFoundersSolver exactESolver( inputMat, numFounders, cnt );
    int res = exactESolver.Solve();
//    int res = exactESolver.SolveDFS();
    if( res >=0 &&  res < cnt)
    {
        cout << "\nMinimum crossover breakpoints = " << res << endl;
        FOUNDER_LIST founderExact;
//        exactESolver.GetSolFounders(founderExact);
        exactESolver.GetDFSSolFounders(founderExact);

        // dump out
        cout << "Exact founders are: \n";
        DumpVecSequences( founderExact );
        cout << "Founder parse are: \n";
        DumpParseMatrixByFounders( founderExact, inputMat);
    }
    else
    {
        // We did not find better solution
        cout << "\nNo better solution found. The optimal solution is the heuristic solution.\n";
        // dump out
        //cout << "Fast founders are: \n";
        //DumpVecSequences( founderFast );
        //cout << "Fast founder parse are: \n";
        //DumpParseMatrixByFounders( founderFast, inputMat);
    }
    // how long does it take?
    cout << "Elapsed time = " << GetElapseTime( tstart1 ) << " seconds." << endl;

    return 0;
}

