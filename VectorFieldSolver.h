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
	void solve(vector<int> & vertices, vector<float> & constraints, 
		vector<int> & constr_fc,
		vector<tuple3f> & constr_fc_dir, 
		VectorField * target );

	//////////////////////////////////////////////////////////////////////////
	// calculate the constraints Vector b in (M+Z)x = b.
	//////////////////////////////////////////////////////////////////////////
	void constraints(vector<int> & vertIds, 
		vector<float> & src_sink_constr, 
		vector<int> & faceIds,
		vector<tuple3f> & face_dir_constr,
		float weight,
		double * b );
	void perturb( vector<int>&  verts, vector<float> & constr );
private:
	pardisoMatrix *mat;
	pardisoSolver *solver;
	oneFormLaplacian *l;

	vector<int> diagonalMatInd;

	vector<double> x;
	vector<double> b;
	/*double * x;
	double * b;*/
};
