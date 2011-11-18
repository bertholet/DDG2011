#pragma once
#include <vector>
#include "tuple3.h"
#include "mesh.h"
#include "pardisoMatCreator.h"
#include "pardisoMatrix.h"

class oneFormLaplacian:public pardisoMatCreator
{
public:
	oneFormLaplacian(std::vector<tuple3i> * fc2he, std::vector<tuple2i> * edges,
		mesh * amesh);

	~oneFormLaplacian(void);

	float val(int i , int j);
	float valdd(int i , int j);
	float val_deltadelta(int i, int j);

	//////////////////////////////////////////////////////////////////////////
	//will store the edge ids with nonzero edge weight in the target vector. 
	//////////////////////////////////////////////////////////////////////////
	void indices(int edge, std::vector<int> & target);

	//////////////////////////////////////////////////////////////////////////
	//will add star d faces.*constr to tge target. target has to have size == nr edges;
	//////////////////////////////////////////////////////////////////////////
	void add_star_d( vector<int> & faces, vector<float> & constr , double * target, int sz);
	
	//////////////////////////////////////////////////////////////////////////
	//will perturb the source sink constraints such that the problem is well
	//posed and the constraints weighed by vornoi area sum to 0.
	//////////////////////////////////////////////////////////////////////////
	void perturb( vector<int>& verts, vector<float> & constr );

	//////////////////////////////////////////////////////////////////////////
	// Adds the additional constaint matrix Z to the matrix.
	// diagonalMatInd are the indices of diagonal elements in pardisoMatrix.a
	// as returned by pardisoMatrix.getDiagElements
	//////////////////////////////////////////////////////////////////////////
	void addZToMat( vector<int> & constr_edges, 
		vector<int> & diagonalMatInd, 
		float weight,
		pardisoMatrix * mat );

	//////////////////////////////////////////////////////////////////////////
	//Removes the additional constraint matrix.
	// diagonalMatInd are the indices of diagonal elements in pardisoMatrix.a
	// as returned by pardisoMatrix.getDiagElements
	//////////////////////////////////////////////////////////////////////////
	void substractZFromMat( vector<int> & constr_edges, 
		vector<int> & diagonalMatInd, 
		float weight,
		pardisoMatrix * mat );

	//////////////////////////////////////////////////////////////////////////
	// Adds the additional directional constraints to target ( target is the
	// b in (M+Z)x=b
	//////////////////////////////////////////////////////////////////////////
	void addZToB( vector<int> & edgeIds, vector<tuple3f> & edge_dir_constr ,
		float weight ,
		double * target, int sz);
private:
	std::vector<tuple3i> * fc2he;
	std::vector<tuple2i> * edges;
	std::vector<std::vector<int>> * v2nbrf;
	std::vector<tuple3i> * faces;
	mesh * myMesh;
};
