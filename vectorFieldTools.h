#pragma once
#include <vector>
#include "tuple3.h"
#include "mesh.h"

class vectorFieldTools
{
public:
	vectorFieldTools(void);
	~vectorFieldTools(void);

	static void oneFormToVector();
	static void vectorToOneForm( tuple3f & dir, 
		int faceNr, vector<tuple3i> & fc2e, 
		vector<tuple2i> & edges, 
		mesh * m, 
		tuple3i & target_edges, 
		tuple3f & target_vals );
};
