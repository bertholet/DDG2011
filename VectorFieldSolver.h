#pragma once
#include "pardiso.h"
#include "pardisoMatrix.h"
#include <vector>
#include "mesh.h"
#include "tuple3.h"
#include "VectorField.h"
#include "oneFormLaplacian.h"
#include "mystatusbar.h"

class VectorFieldSolver
{
public:
	VectorFieldSolver(mesh * aMesh, vector<tuple2i> & edges, vector<tuple3i> & f2he,
		myStatusBar * statusBar);
	~VectorFieldSolver(void);

	//////////////////////////////////////////////////////////////////////////
	// Takes the vertex constraints (constrains[i] = contraint of vertex[i]).
	// whoch denote source and sink vertices and uses them to solve
	// for the vector field.
	// weight is the weight the edge constraints shall get.
	//////////////////////////////////////////////////////////////////////////
	void solve(vector<int> & vertices, vector<float> & constraints, 
		vector<int> & constr_edges,
		vector<tuple3f> & constr_edg_dir, 
		float weight,
		float constrLength,
		VectorField * target );

	//////////////////////////////////////////////////////////////////////////
	//Instead of enforcing vectors of the vector fields only their direction
	//is inforced
	//
	//////////////////////////////////////////////////////////////////////////
	void solveDirectional(vector<int> & vertices, vector<float> & constraints, 
		vector<int> & constr_fc,
		vector<tuple3f> & constr_fc_dir, 
		float weight,
		float constrLength,
		VectorField * target );

	//////////////////////////////////////////////////////////////////////////
	// In a first step, before directional constraints are introduced the length
	// of the vectors is estimated
	//
	//////////////////////////////////////////////////////////////////////////

	void solveLengthEstimated(vector<int> & vertices, vector<float> & src_snk_constraints, 
		vector<int> & constr_fc,
		vector<tuple3f> & constr_fc_dir, 
		float weight,
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

	//////////////////////////////////////////////////////////////////////////
	// face based constraint calculation with precalculated lengths
	//
	//////////////////////////////////////////////////////////////////////////
	void constraints(vector<int> & vertIds, 
		vector<float> & src_sink_constr, 
		vector<int> & faceIds, 
		vector<tuple3f> & face_dir_constr,
		vector<float> & lengths,
		float weight, double * b );

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

	void addDirConstraint2Mat( vector<int> & constr_faces ,
		vector<tuple3f> & constr_face_dir, 
		float weight, 
		pardisoMatrix * target );
	void constraintsSrcSinkOnly(vector<int> & vertIds, 
		vector<float> & src_sink_constr, 
		double * b);
	void findLengths( vector<int> & vertIDs, vector<float> & src_sink_constr, 
		vector<int> & constr_fc, 
		vector<float> & target_lengths );
	void pushEdges( vector<int> & constr_fc, vector<tuple3i> & f2e , vector<int>  & target );
};
