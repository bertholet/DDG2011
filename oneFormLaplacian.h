#pragma once
#include <vector>
#include "tuple3.h"
#include "mesh.h"

class oneFormLaplacian
{
public:
	oneFormLaplacian(std::vector<tuple3i> * fc2he, std::vector<tuple2i> * edges,
		std::vector<std::vector<int>> * nbr_fcs ,
		std::vector<tuple3i> * faces,
		mesh * amesh);
	~oneFormLaplacian(void);

	float val(int i , int j);
	float valdd(int i , int j);
	float val_deltadelta(int i, int j);

	//will store the edge ids with nonzero edge weight in the target vector. 
	void indices(int edge, std::vector<int> & target);

private:
	std::vector<tuple3i> * fc2he;
	std::vector<tuple2i> * edges;
	std::vector<std::vector<int>> * v2nbrf;
	std::vector<tuple3i> * faces;
	mesh * myMesh;
};
