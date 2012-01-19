#pragma once
#include <vector>
#include "tuple3.h"
#include "oneForm.h"

class fluidTools
{
public:
	fluidTools(void);
	~fluidTools(void);

	static void flux2Velocity(oneForm flux, std::vector<tuple3f> & bariCoords, std::vector<tuple3f> & target);
};
