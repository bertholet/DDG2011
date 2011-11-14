#pragma once
#include <vector>
#include "tuple3.h"
#include "mesh.h"
#include "pardisoMatCreator.h"

class oneFormLaplacian:public pardisoMatCreator
{
public:
	oneFormLaplacian(std::vector<tuple3i> * fc2he, std::vector<tuple2i> * edges,
		mesh * amesh);

	~oneFormLaplacian(void);

	float val(int i , int j);
	float valdd(int i , int j);
	float val_deltadelta(int i, int j);

	//will store the edge ids with nonzero edge weight in the target vector. 
	void indices(int edge, std::vector<int> & target);

	//will store star d faces.*constr in target. target has to have size == nr edges;
	void stard( vector<int> & faces, vector<float> & constr , double * target, int sz);
	void perturb( vector<int>& verts, vector<float> & constr );
private:
	std::vector<tuple3i> * fc2he;
	std::vector<tuple2i> * edges;
	std::vector<std::vector<int>> * v2nbrf;
	std::vector<tuple3i> * faces;
	mesh * myMesh;
};
