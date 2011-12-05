#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <string>
#include "pardisoMatCreator.h"
#include "mystatusbar.h"


class pardisoMatrix
{
public:
	//indices of the values in the sparse matrix
	std::vector<int> ia, ja;
	//values of the elements of the sparse matrix
	std::vector<double> a;

	pardisoMatrix(void);
	~pardisoMatrix(void);

	void initMatrix(pardisoMatCreator & creator, int dim, myStatusBar * bar = NULL);

	int dim();
	void saveMatrix(std::string file);
	void getDiagonalIndices( std::vector<int> & target_ind );

	//////////////////////////////////////////////////////////////////////////
	// IF and only IF mat has an entry at i,j, val is added.
	// i, j in ZERO based notation
	// ELSE an assertion will fail and an error be thrown
	//////////////////////////////////////////////////////////////////////////
	void add( int i, int j, float val );
};
