#ifndef SNP_UTILS_H
#define SNP_UTILS_H

#include "Utils.h"
#include "Utils2.h"
#include "BinaryMatrix.h"
#include "UnWeightedGraph.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// important data structure

class SNPDistInfo
{
public:
    SNPDistInfo( );
    ~SNPDistInfo();

    // Now read in from a file
    bool ReadFromFile( const char* fileName );
    void SetRange( int range ) { totalRange = range; }
    int GetRange() {return  totalRange;}
    int GetSNPNum();
    int GetSNPLoc( int i );
    void InitDefaultDistance(int numSNPs);


private:


    vector<int> phyLocSNPs;
    int totalRange;

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern SNPDistInfo SNPDefaultDistInfo;




#endif  // ASSOCIATION_TEST_H

