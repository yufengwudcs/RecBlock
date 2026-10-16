#include <sstream>
#include "SNPUtils.h"
#include <cmath>

//*****************************************************************************************************

SNPDistInfo SNPDefaultDistInfo;


//*****************************************************************************************************
// THis is the class that mainly control how we can solve the problem

SNPDistInfo :: SNPDistInfo( )
{
}

SNPDistInfo :: ~SNPDistInfo()
{
}


// Now read in from a file
bool SNPDistInfo :: ReadFromFile( const char* fileName )
{
	// Now read in configuration file 
	ifstream distFile( fileName  );
	if(distFile.is_open()  == false)
	{
		// alert
        return false;
	}    

    // Now start to read in the distance
    phyLocSNPs.clear();

    // Read in a single line from it
    char buf[10240];
    distFile.getline(buf, 10240);
	distFile.close();

    // Check to see if the line contains all numbers
    for(unsigned int i=0; i<strlen(buf); ++i)
    {
//        if(buf[i] != ' ' && buf[i] != ',' && ( buf[i] < '0' || buf[i] >'9' ) )
        if( (buf[i] >= 'a' &&  buf[i] <= 'z')  ||  (buf[i] >='A' && buf[i] <= 'Z' )  )
        {
            // not a good distance
//cout << "First line does not contain all numbers.\n";
            distFile.close();
            return false;
        }
    }

    // Look like good distance here
    istringstream strStream( buf );

//    while(  strStream.eof() == false  )
//    {
    int dist;
    while( strStream >> dist)
    {
        phyLocSNPs.push_back( dist );
//        cout << "Read in one distance: " << dist << endl;

    }

    // close the file
    distFile.close();

    // Finish 
    return true;
}

int SNPDistInfo :: GetSNPNum()
{
    return phyLocSNPs.size();
}

int SNPDistInfo :: GetSNPLoc( int i ) 
{
    YW_ASSERT_INFO(  i < GetSNPNum(), "Range exceeded in SNP" );
    return phyLocSNPs[i];
}

//*****************************************************************************************************
void SNPDistInfo :: InitDefaultDistance(int numSNPs)
{
    phyLocSNPs.clear();
    // We create a default distance of very large size, say 10000 sites
    for(int i=0; i<numSNPs; ++i)
    {
        phyLocSNPs.push_back ( i );
    }
}


