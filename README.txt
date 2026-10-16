                             RecBlock v1.0 Beta

Citation: Yufeng Wu and Dan Gusfield, Improved Algorithms for Inferring the Minimum Mosaic of a Set of Recombinants, CPM 2007, pp. 150 to 161.

INTRODUCTION :  This program, RecBlock, is a tool for reconstructing mosaic structure
		of a given set of SNP sequences. Due to recombination, a set of 
		sequences in a population form a mosaic strucgture, where recombination
		breakpoints breaks the sequences into blocks. For example, consider
		the following example, with 8 sequences.

			000101
			110000
			101011
			000001
			100000
			101110
			100100
			010010
 
		Suppose we assume these 8 sequences are decendents of THREE founder
		sequences. And we want to find out which set of three founder
		sequences gives the SMALLEST number of possible breakpoints
		among ALL choices of three founder sequences. The following
		set of founder sequences gives the best solution:
		10 breakpoints are the MINIMUM.

			Founders sequences
			000101		A
			010000		B		
			101110		C

			Minimum Mosaic
			AAAAAA
			CBBBBB
			CCCBCA
			AAABBA
			CCBBBB
			CCCCCC
			CCAAAB
			BBBBCC

		RecBlock has two operating mode: exact mode and approximate mode.
		The exact mode ensures the outputed mosaic is the minimum.
		But the computation can be slow. Our experience shows that
		RecBlock is very fast for two or three founders, and reasonably
		fast with four founders for moderate-sized data (50 rows by 50 sites).
		RecBlock can solve an instance with 20 sequences and 32 sites
		and with five founders optimally within a few minimutes.
		The exact mode runs in TWO passes. The first pass is to
		search for a quick (but possibly sub-optimal) solution,
		and the second pass searches for the optimal solution
		(which could be slow if the data size/number of founders are 
		large).

		When the data size grows or the number of founders increases,
		we recommend to turn on approximate mode by either -D and/or
		-C option. For example, -D0 is the fastest but may give 
		sub-optimal results, since it takes a greedy approach. 
		Use -C500 (e.g.) to reduce the number of active search paths.
		The approximate mode can handle quite large data and 6-8
		founders and runs reasonably fast. We note that the
		approximate mode often gives the optimal solutions. There
		are indications that this heuristic method gives quite
		strong results.

                RecBlock is tested on Linux machines, as well as Windows 
                (with cygwin).  

---------------------------------------------------------------------------

SYNOPSIS: ./RecBlock -Fk [-Ck] [-Dk ] data-filename

OPTIONS:
  -F		Specify the number of founders. 

  -C		Turn on the approximate mode by restricting the number
		of active search paths to k. 

  -D		Turn on the approximate mode by restricting the number
   		of kept search path to be within k from the current best
		path. For example, if a search path currently needs
		10 breakpoints and the currently the best path needs
		9 bkpts. If -D0 is specified, this path is dropped.
		If -D1 is specified, this path is kept for further 
		exploration. 

---------------------------------------------------------------------------

DATA FILE    :  The first line of the data file is IGNORED.  If you like,
                you may put a description of the data there.
                The data should be in 0,1. (White space is allowed 
                between columns.)  Each sequence should be placed in a its
                own row. No SPACE between two values, please. 


EXAMPLE      :  An example data set (simple-example.dat) is included.
                Try running "./RecBlock -F3 simple-example.dat" to 
                check that everything works correctly.
		   
CONTACT      :  Please send bug reportss and technical questions to Yufeng Wu at 
                <yfwu@ucdavis.edu>.


