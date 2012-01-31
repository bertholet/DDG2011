#pragma once
#include "meshMetaInfo.h"
#include <vector>

class nullForm
{
private:
	//mesh the 1Form is living on
	meshMetaInfo * m;
	std::vector<double> form;
public:
	nullForm(meshMetaInfo & mesh);
	~nullForm(void);
	int size();
	std::vector<double> & getVals();
	// returns value times orientation (+-1) forces
	//user to think about the orientation at call.
	double get( int i, int orientation );

	meshMetaInfo * getMesh(){return m;}
};
