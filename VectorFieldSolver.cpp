#include "VectorFieldSolver.h"


VectorFieldSolver::VectorFieldSolver(mesh * aMesh, vector<tuple2i> & edges, vector<tuple3i> & f2he)
{
	l = new oneFormLaplacian(&f2he,&edges,aMesh);
	mat = new pardisoMatrix();
	mat->initMatrix(*l, edges.size());

	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat, 1);

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

void VectorFieldSolver::solve(vector<int> & vertIDs, vector<float> & src_sink_constr, VectorField * target )
{
	constraints(vertIDs, src_sink_constr, b);
	solver->solve(x,b);

	for(int i = 0; i < mat->dim(); i++){
		target->setOneForm(i,1,(float) x[i]); //solved for the edges as they are oriented.
	}
}

void VectorFieldSolver::constraints(vector<int> & vertIds, vector<float> & constr, double * b )
{
	for(int i = 0; i < mat->dim(); i++){
		b[i] = 0;
	}
	l->stard(vertIds, constr, b, mat->dim());

}
