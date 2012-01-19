#pragma once
#include <vector>
#include "tuple3.h"
#include "oneForm.h"

class fluidTools
{
public:
	fluidTools(void);
	~fluidTools(void);

	//////////////////////////////////////////////////////////////////////////
	// the one form on an edge is assumed to be the quantity you get by integrating
	// a flow along the edge times the normal on the edge- i.e. the 
	// flow passing the edge i.e. the flux. This now translates flux
	// back to a velocity vector.
	//////////////////////////////////////////////////////////////////////////
	static void flux2Velocity(oneForm flux, std::vector<tuple3f> & bariCoords, std::vector<tuple3f> & target);

	//////////////////////////////////////////////////////////////////////////
	// walk the path for at most a timestep t in the given triangle. t is updated
	// to the remaining t to go, pos is updated and the new actual triangle
	// if the border of the triangle is reached.
	//////////////////////////////////////////////////////////////////////////
	static void walkPath(tuple3f * pos, int * triangle, float *  t);

	static void vorticity2flux();
};
