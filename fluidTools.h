#pragma once
#include <vector>
#include "tuple3.h"
#include "oneForm.h"
#include "meshMetaInfo.h"
#include "oneForm.h"
#include "twoForm.h"
#include "nullForm.h"

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


	static void vorticity2flux(nullForm & vorticity, oneForm & flux);	
	

	//////////////////////////////////////////////////////////////////////////
	// generalized baricentric coordinates for convex polytopes. Will be store
	// in target.
	//////////////////////////////////////////////////////////////////////////
	static void bariCoords(tuple3f & point, int dualFace_id, std::vector<tuple3f> & dualVert_pos, 
		std::vector<float> & target, meshMetaInfo & mesh);

	//////////////////////////////////////////////////////////////////////////
	// Everything is flattened along the curvature Normal
	//////////////////////////////////////////////////////////////////////////
//	static void flattenedBariCoords(tuple3f & point, int dualFace, std::vector<tuple3f> & dualVert_pos,
//		std::vector<float> & target, meshMetaInfo & mesh);


	
private:
	//////////////////////////////////////////////////////////////////////////
	// Help Function to calculate the baricentric coordinates.
	//////////////////////////////////////////////////////////////////////////
	static float bariWeight(tuple3f & point , int nr, int dualFace_id, std::vector<int> & dualVert_ids, 
			std::vector<tuple3f> & dualVert_pos, meshMetaInfo & mesh);
	//////////////////////////////////////////////////////////////////////////
	// Help method that returns the dot number +- 0.0000001 such that it is 
	// at least that much different from 0.
	//////////////////////////////////////////////////////////////////////////
	static inline float nonzeroDot(tuple3f & n2, tuple3f & pos);

};
