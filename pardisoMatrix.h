#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <string>
#include "pardisoMatCreator.h"


class pardisoMatrix
{
public:
	//indices of the values in the sparse matrix
	std::vector<int> ia, ja;
	//values of the elements of the sparse matrix
	std::vector<double> a;

	pardisoMatrix(void);
	~pardisoMatrix(void);

	void initMatrix(pardisoMatCreator & creator, int dim);

	int dim();
	void saveMatrix(std::string file);

};
