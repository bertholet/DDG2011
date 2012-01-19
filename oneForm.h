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
};
