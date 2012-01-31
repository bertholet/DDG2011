#pragma once
#include "meshMetaInfo.h"
#include <vector>

class oneForm
{
private:
	//mesh the 1Form is living on
	meshMetaInfo * m;
	std::vector<double> form;

public:
	oneForm(meshMetaInfo & mesh);
	~oneForm(void);
	int size();
	std::vector<double> & getVals();
	// returns value times orientation (+-1) forces
	//user to think about the orientation at call.
	double get( int i, int orientation );

	meshMetaInfo * getMesh(){return m;}
	//sets form[edge] = val*orientation
	void set( int edge, double val, int orientation );
};
