#include "fluidTools.h"

fluidTools::fluidTools(void)
{
}

fluidTools::~fluidTools(void)
{
}

void fluidTools::flux2Velocity( oneForm flux, std::vector<tuple3f> & bariCoords, std::vector<tuple3f> & target )
{
	assert(target.size() == flux.size());

}
