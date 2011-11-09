#pragma once
#include <vector>

class oneFormLaplacian
{
public:
	oneFormLaplacian(vector<tuple3i> & fc2he, vector<tuple2i> & edges );
	~oneFormLaplacian(void);

	int val(int i , int j)
};
