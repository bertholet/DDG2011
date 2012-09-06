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

	//////////////////////////////////////////////////////////////////////////
	// Get Dual Vertex Positions, circumcentric. These points are stored in target.
	//additionally dual vertices on the mesh boundary are computed. the dualface2vertex
	//relationship is stored in targetdualF2V
	//////////////////////////////////////////////////////////////////////////
	static void getDualVertices_border( meshMetaInfo & mesh, std::vector<tuple3f> & target, std::vector<std::vector<int>> & targetdualF2V );

};
