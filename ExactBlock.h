#ifndef EXACT_BLOCK_H
#define EXACT_BLOCK_H

#include "Utils.h"
#include "Utils2.h"
#include "Utils3.h"
#include "BinaryMatrix.h"
#include "GenotypeMatrix.h"
#include "UnWeightedGraph.h"
#include <queue>
using namespace std;

#include "BlockUtils.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetApproxLimitControl(int nGreedyBkptLimit);
void SetMaxConfigNumStep( int nMaxConfig);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// important data structure

typedef int FOUNDER_ENCODED_VAL;


class ExactBlockSolver
{
public:
    ExactBlockSolver( const BinaryMatrix &mat, int numFounders, int upperBound );
    ~ExactBlockSolver();

    // Now read in from a file
    int Solve();
    void GetSolFounders( FOUNDER_LIST &founders );

private:
    // Use an integer to encode a founder bit (say 00110)
    typedef int ENCODED_FOUNDER_BIT;

    // Implementations
    void Preprocess();
    int CalcIVLowerBound();
    void GetFounderFromBits( FOUNDER_LIST & foundersIV,  const vector<int> &founderBitChoice, 
        const vector<int> &founderBits );
    bool IsFounderSegmentsOptimal( const INTERVAL &ivToCheck, const FOUNDER_LIST &foundersIV  );
    
    // private data
    const BinaryMatrix &inputMat;
    int numFounders;
    int knownUpperBound;
    FOUNDER_LIST foundersBest;

    // Store a list of all possible founder bits for intervals
    map<INTERVAL, vector< vector<FOUNDER_ENCODED_VAL> > > mapIVFounderPotBits;

    // Maximum number of crossovers for an interval for intervals
    map<INTERVAL, int> ivLocalBounds;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Useful functions



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Another way of computing exact minimum crossovers
// The idea is to keep track of the minimum positions of each INPUT rows (note not founder rows) in the founder

// This class offer a configuration for a site in the founders
struct FounderSiteConfig
{
public:
    FounderSiteConfig();
    void Dump() const;
    bool operator==(const FounderSiteConfig &rhs) const;
    int GetMinBreakpoints() const;
    int CalcRelativeScore(const FounderSiteConfig &rhs)  const;
    int CalcRelativeScorePerm(const FounderSiteConfig &rhs) const;
    int CalcRelativeScorePermApprox(const FounderSiteConfig &rhs) const;
    int CalcRelativeScorePermGreedy(const FounderSiteConfig &rhs) const;
    bool IsContained( const FounderSiteConfig &rhs ) const;
    void Normalize(FounderSiteConfig &fcNormal) const;
    bool IsLeftConfig( const FounderSiteConfig &fcRIght, const BinaryMatrix &inputMat) const;
    bool IsLeftConfig( const FounderSiteConfig &fcRIght, const GenotypeMatrix &inputMat, 
        vector<bool> &zeroTopFlags) const;
    void Permuate(const vector<int> &indMapping);
    bool IsIsomorphismTestOK( const vector<int> &founderState) const;
    void InitUnrestrictedUniBlock(const vector<int> &founderState);
    void InitRestrictedUniBlock( const vector<int> &founderState, const FounderSiteConfig& prevFounderConfig) ;
    int GetNumInputRows() const {return inRowsMinPosInFounders.size();}
    void InitMultiplicity(vector<int> *pR) {
        // for now, do not use multiplicty
        //pRowMultiplicity = pR;
    }


    // Data
    int site;                                       // which site it is in for the founder
    vector<int> founderStates;                      // value of founder at each position
    vector< set<int> > inRowsMinPosInFounders;      // positions where each row ends in founder as minimum
    vector<int> numBreakpointsRows;
    vector<int> listUniRowBlocks;                   // in config, the segments of identical rows
    vector<int> *pRowMultiplicity;

public:
    bool IsLeftConfigAtRow( const FounderSiteConfig &fcRight, int r, const set<int> &leftRowSet, 
        const set<int> &rightRowSet ) const;

};


// Define an abstract class as base 
class ExactEnumFoundersAbstractSolver
{
public:
    ExactEnumFoundersAbstractSolver( );
    virtual ~ExactEnumFoundersAbstractSolver() {}
    void SetDispFlag(bool f) {fDispInfo = f;}
    void SetApproxFlag( bool f) {fApproxFlag = f;}
    virtual int SolveDFS();
    void GetDFSSolFounders( FOUNDER_LIST &founders ) {founders = optDFSFounders; }

protected:
    virtual void GetUnRestrictedFounderConfigAt(int site, const vector<int> &founderState, 
        FounderSiteConfig& founderConfig) = 0;
    int GetMinBreakpoints( const vector<FounderSiteConfig> &prevSiteFounderConfigs, 
        vector<int> *pIndList = NULL, bool fPerm = false);
    void FindFounderConfigs( int site, const vector<int> &founderState, 
                        const vector<FounderSiteConfig> &prevSiteFounderConfigs, 
                        vector<FounderSiteConfig> &curFoundersConfigs );
    void FindMatchedFounderRows( int bitVal, const vector<int> &founderState, set<int> &fRows );

    virtual void BuildFounderFromPre( int site, const FounderSiteConfig &founderConfigPre, 
        const vector<int> &founderState, vector<FounderSiteConfig> &fConfig  ) = 0 ;
    virtual void ScreenAppendConfig( vector<FounderSiteConfig> &stepFConfigs, 
        const vector<FounderSiteConfig>& fConfigList ) ;
    virtual void PruneFounderConfig(vector<FounderSiteConfig> &foundersConfigs) = 0;
    virtual void PruneFounderConfigStep(vector<FounderSiteConfig> &foundersConfigs, vector<int> &keptPos);
    virtual void PruneFounderConfigStepSwap(vector<FounderSiteConfig> &foundersConfigs) = 0;
    virtual void PruneFounderConfigStepPerm(vector<FounderSiteConfig> &foundersConfigs) ;
    virtual bool IsOptPossibleWithPartialSol( int  start, int end, int bd ) = 0;
    virtual int GetDataWidth() = 0;
    virtual int GetNumFounders() = 0;
    virtual int GetUpperBound() = 0;
    virtual void SetUpperBound(int ub) = 0;
    virtual int GetMultiplicity( int r );
    //void UnrestrictedUniBlock(const vector<int> &founderState, FounderSiteConfig& founderConfig) const;
    //bool IsIsomorphismTestOK(const FounderSiteConfig& prevFounderConfig, 
    //    const vector<int> &founderState) const;
    //void RestrictedUniBlock(const FounderSiteConfig& prevFounderConfig, 
    //    const vector<int> &founderState, FounderSiteConfig& founderConfig) const;
    void ReduceConfigs(  );

    // a set of functions supporting DFS
    void InitDFS(int numSites);
    void MarkExplored(  const FounderSiteConfig *pFCExplored  );
    const FounderSiteConfig *GetNextToExplore(int site  );
    bool AreMoreFCToExplore(int site);
    void AppendFCToExplore( int site, const vector<FounderSiteConfig> &listFCToExp);
    bool IsFCExcluded( const FounderSiteConfig &fc );
    void BuildDFSSolFounders(  );
    virtual bool IsNgbrPairCFs(const FounderSiteConfig &leftCF, const FounderSiteConfig &rightCF ) = 0;
    void RetrieveRandFounders( FOUNDER_LIST &rndFounders, vector< FounderSiteConfig > &rndDFSFCs );

    // data members shared by sub-classes
    bool fDispInfo;
    bool fApproxFlag;
    vector< vector<FounderSiteConfig>  > unexploredFCStore;
    vector< vector<FounderSiteConfig> >  exploredFCStore;
    FOUNDER_LIST optDFSFounders;
    vector<FounderSiteConfig> optDFSFCs;

    // Here are for more trials on the solution by storing additional solutions here
    //int randSavedSolNum;
    //vector< FOUNDER_LIST > listAdditionalFounders;
    //vector< vector<FounderSiteConfig>  > listAddOptDFSFCs;
};


// This class searchs and finds the best founders
class ExactEnumFoundersSolver : public ExactEnumFoundersAbstractSolver
{
public:
    ExactEnumFoundersSolver( const BinaryMatrix &mat, int numFounders, int upperBound );
    virtual ~ExactEnumFoundersSolver();

    // Now read in from a file
    int Solve();
    void GetSolFounders( FOUNDER_LIST &founders );
    int SolveTestCode();    // temp test code

private:
    // implementations
    void GetUnRestrictedFounderConfigAt(int site, const vector<int> &founderState, FounderSiteConfig& founderConfig);
    void BuildFounderFromPre( int site, const FounderSiteConfig &founderConfigPre, 
        const vector<int> &founderState, vector<FounderSiteConfig> &fConfig  );
    bool IsFounderBetterThan( const FounderSiteConfig & fConfig1, const FounderSiteConfig &fConfig2 ) ;
    bool IsOptPossibleWithPartialSol( int  start, int end, int bd );
    void PruneFounderConfig(vector<FounderSiteConfig> &foundersConfigs);
//    void PruneFounderConfigStep(vector<FounderSiteConfig> &foundersConfigs, vector<int> &keptPos);
    void PruneFounderConfigStepSwap(vector<FounderSiteConfig> &foundersConfigs);
//    void PruneFounderConfigStepPerm(vector<FounderSiteConfig> &foundersConfigs);
    void FindSolFounders( const vector< vector<FounderSiteConfig> > &listFounderSitesConfig );
    virtual int GetDataWidth() ;
    int GetNumFounders() {return numFounders; }
    int GetUpperBound() {return knownUpperBound;}
    void SetUpperBound(int ub) {knownUpperBound = ub;}
    virtual bool IsNgbrPairCFs(const FounderSiteConfig &leftCF, const FounderSiteConfig &rightCF ) ;
    virtual int GetMultiplicity( int r );

    // data
    BinaryMatrix inputMat;
    int numFounders;
    int knownUpperBound;
    FOUNDER_LIST optFounders;

    // Maximum number of crossovers for an interval for intervals
    map<INTERVAL, int> ivLocalBounds;
    // This list contains min-breakpoints for the site up till then
    vector<int> minBkptsForSites;

    // multiplicity of rows
    vector<int> vecRowsMultiplicity;

};


// For genotype hap founder solver
class ExactEnumGenoFoundersSolver : public ExactEnumFoundersAbstractSolver
{
public:
    ExactEnumGenoFoundersSolver( const GenotypeMatrix &mat, int numFounders );
    virtual ~ExactEnumGenoFoundersSolver();

    // Now read in from a file
    int Solve();
    void GetDFSPhasedMatrix( BinaryMatrix &res, FOUNDER_LIST &founders );
    void GetPhasedMatrix( BinaryMatrix &res, FOUNDER_LIST &founders );


private:
    // impelemtnations
    void GetUnRestrictedFounderConfigAt(int site, const vector<int> &founderState,
                                                               FounderSiteConfig& founderConfig);
//    void FindFounderConfigs( int site, const vector<int> &founderNPState, 
//                        const vector<FounderSiteConfig> &prevSiteFounderConfigs, 
//                        vector<FounderSiteConfig> &curFoundersConfigs );
    virtual void BuildFounderFromPre( int site, const FounderSiteConfig &founderConfigPre, 
        const vector<int> &founderState, vector<FounderSiteConfig> &fConfig  ) ;
    virtual void PruneFounderConfig(vector<FounderSiteConfig> &foundersConfigs) ;
//    virtual void PruneFounderConfigStep(vector<FounderSiteConfig> &foundersConfigs, vector<int> &keptPos) ;
    virtual void PruneFounderConfigStepSwap(vector<FounderSiteConfig> &foundersConfigs) ;
    virtual void PruneFounderConfigStepPerm(vector<FounderSiteConfig> &foundersConfigs) ;
    bool IsOptPossibleWithPartialSol( int  start, int end, int bd );
    int QuickTest2( int r, int site, const FounderSiteConfig& preFC, const vector<int> &founderState  );
    virtual int GetDataWidth() ;
    int GetNumFounders() {return numFounders;}
    int GetUpperBound() {return knownUpperBound;}
    void SetUpperBound(int ub) {knownUpperBound = ub;}
    virtual bool IsNgbrPairCFs(const FounderSiteConfig &leftCF, const FounderSiteConfig &rightCF ) ;
    void ParseInputMat(const vector<FounderSiteConfig> &);
    void ReduceData();
    void FinishSolData(  const BinaryMatrix &partialResult   );
    bool IsSiteRecentMut(int site );
    // find founders/phases WITHOUT DFS
    void FindSolFounders( const vector< vector<FounderSiteConfig> > &listFounderSitesConfig );
    

    // data
    const GenotypeMatrix &inputMat;
    GenotypeMatrix reducedGenoMat;
    int numFounders;
    int knownUpperBound;
    FOUNDER_LIST optFounders;
    BinaryMatrix phasedMat;
    // This list contains min-breakpoints for the site up till then
    vector<int> minBkptsForSites;
    vector<int> posFirst2s;
    map<INTERVAL, int> mapIVLBs;
    // create a list of 2s whose phase is determined
    bool fReductionEnabled;
    vector<map< pair<int,int>, bool > > presetPhasesForRows;

};


#endif  // EXACT_BLOCK_H

