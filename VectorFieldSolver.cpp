#include "VectorFieldSolver.h"


VectorFieldSolver::VectorFieldSolver(mesh * aMesh, vector<tuple2i> & edges, vector<tuple3i> & f2he)
{
	l = new oneFormLaplacian(&f2he,&edges,aMesh);
	mat = new pardisoMatrix();
	mat->initMatrix(*l, edges.size());

	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat, 1);
	mat->getDiagonalIndices(this->diagonalMatInd);

	x= new double[mat->dim()];
	b = new double[mat->dim()];
	//solver->solve();
}

VectorFieldSolver::~VectorFieldSolver(void)
{
	delete solver;
	delete mat;
	delete[] x;
	delete[] b;
	delete l;
}

void VectorFieldSolver::solve(vector<int> & vertIDs, 
			vector<float> & src_sink_constr, 
			vector<int> & constr_fc,
			vector<tuple3f> & constr_fc_dir, 
			VectorField * target )
{
	constraints(vertIDs, src_sink_constr, constr_fc, constr_fc_dir, b);

	//want to store the oneform laplacian matrix M between two
	//calls.... this means it has to be tidied up 
	//after having added the extra constraints
	l->addZToMat(constr_fc, diagonalMatInd, mat);
	solver->solve(x,b);
	l->substractZFromMat(constr_fc, diagonalMatInd, mat);

	for(int i = 0; i < mat->dim(); i++){
		target->setOneForm(i,1,(float) x[i]); //orientation = 1: solved for the edges as they are oriented.
	}
}

void VectorFieldSolver::constraints(vector<int> & vertIds, 
			vector<float> & src_sink_constr, 
			vector<int> & faceIds,
			vector<tuple3f> & face_dir_constr,
			double * b )
{
	for(int i = 0; i < mat->dim(); i++){
		b[i] = 0;
	}
	l->addZToB(faceIds, face_dir_constr,b, mat->dim());
	l->stard(vertIds, src_sink_constr, b, mat->dim());

}

void VectorFieldSolver::perturb( vector<int>& verts, vector<float> & src_sink_constr )
{
	l->perturb(verts, src_sink_constr);
}
