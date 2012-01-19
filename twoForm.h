#pragma once
#include "meshMetaInfo.h"
#include <vector>

class twoForm
{
private:
	meshMetaInfo * m;
	std::vector<double> form;
public:
	twoForm(meshMetaInfo & mesh);
	~twoForm(void);
};
