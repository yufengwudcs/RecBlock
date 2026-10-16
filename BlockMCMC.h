#ifndef BLOCK_MCMC_H
#define BLOCK_MCMC_H

#include "Utils.h"
#include "Utils2.h"
#include "BinaryMatrix.h"
#include "UnWeightedGraph.h"
#include "BlockUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// important data structure

class BlockMCMCMain
{
public:
    BlockMCMCMain( const BinaryMatrix &mat, int numFounders );
    ~BlockMCMCMain();

    // Run MCMC for certain steps
    int Run( int steps );

private:
    // Data type
    typedef vector< vector<int> > BLOCK_MCMC_STATE;

    // Implementation
    void Init();
    void InitHeurstic();
    void Update();
    double CalcScore( const BLOCK_MCMC_STATE &state);
    double CalcRelativeScore( double scoreNew, double scoreOld);
    void Dump();

    // Private data
    const BinaryMatrix &inputMat;
    int numFounders;
    BLOCK_MCMC_STATE curFounderStates; // for each bit in the data, which founder it comes from
    double curScore;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////




#endif  // ASSOCIATION_TEST_H

