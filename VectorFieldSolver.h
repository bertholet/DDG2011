#pragma once
#include "pardiso.h"
#include "pardisoMatrix.h"
#include <vector>
#include "mesh.h"
#include "tuple3.h"
#include "VectorField.h"
#include "oneFormLaplacian.h"

class VectorFieldSolver
{
public:
	VectorFieldSolver(mesh * aMesh, vector<tuple2i> & edges, vector<tuple3i> & f2he);
	~VectorFieldSolver(void);

	//////////////////////////////////////////////////////////////////////////
	// Takes the vertex constraints (constrains[i] = contraint of vertex[i]).
	// whoch denote source and sink vertices and uses them to solve
	// for the vector field.
	//////////////////////////////////////////////////////////////////////////
	void solve(vector<int> & vertices, vector<float> & constraints, VectorField * target );
	void constraints(vector<int> & vertIds, vector<float> & constr, double * b );
private:
	pardisoMatrix *mat;
	pardisoSolver *solver;
	oneFormLaplacian *l;

	double * x;
	double * b;
};
