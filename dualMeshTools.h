#pragma once
#include <vector>
#include "meshMetaInfo.h"

class dualMeshTools
{
public:
	dualMeshTools(void);
	~dualMeshTools(void);

	//////////////////////////////////////////////////////////////////////////
	// Get Dual Vertex Positions, circumcentric. These points are stored in target.
	//////////////////////////////////////////////////////////////////////////
	static void getDualVertices(meshMetaInfo & mesh, std::vector<tuple3f> & target);

};
