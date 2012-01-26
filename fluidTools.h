#pragma once
#include <vector>
#include "tuple3.h"
#include "oneForm.h"
#include "meshMetaInfo.h"

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
	static void flux2Velocity(oneForm & flux, std::vector<tuple3f> & target, meshMetaInfo & mesh);


	static void vorticity2flux();	
	
	//////////////////////////////////////////////////////////////////////////
	// walk the path for at most a timestep t in the given triangle. t is updated
	// to the remaining t to go, pos is updated and the new actual triangle
	// if the border of the triangle is reached.
	//////////////////////////////////////////////////////////////////////////
	static void walkPath(tuple3f * pos, int * triangle, float *  t);



	//////////////////////////////////////////////////////////////////////////
	// generalized baricentric coordinates for convex polytopes.
	//////////////////////////////////////////////////////////////////////////
	static void bariCoords(int dualFace_id, std::vector<tuple3f> & dualVert_pos, 
		std::vector<float> & target, meshMetaInfo & mesh);

private:
	//////////////////////////////////////////////////////////////////////////
	// Help Function to calculate the baricentric coordinates.
	//////////////////////////////////////////////////////////////////////////
	static float bariWeight(int nr, int dualFace_id, std::vector<int> & dualVert_ids, 
			std::vector<tuple3f> & dualVert_pos, meshMetaInfo & mesh);
	//////////////////////////////////////////////////////////////////////////
	// Help method that returns the dot number +- 0.0000001 such that it is 
	// at least that much different from 0.
	//////////////////////////////////////////////////////////////////////////
	static inline float nonzeroDot(tuple3f & n2, tuple3f & pos);

	//////////////////////////////////////////////////////////////////////////
	// helpmethod that interpolates the velocityfield defined on the dualvertex
	// positions.
	//////////////////////////////////////////////////////////////////////////
	static void getVelocity(tuple3f & pos, tuple3i & tr, 
		std::vector<tuple3f> & velocities,
		std::vector<tuple3f> & dualVert_pos,
		meshMetaInfo & mesh):

};
