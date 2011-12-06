#include "VectorFieldSolver.h"
#include "Model.h"


VectorFieldSolver::VectorFieldSolver(mesh * aMesh, vector<tuple2i> & edges, vector<tuple3i> & f2he,
									 myStatusBar * statusBar)
{
	l = new oneFormLaplacian(&f2he,&edges,aMesh);
	mat = new pardisoMatrix();
	mat->initMatrix(*l, edges.size(), statusBar);

	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat, 1);
	mat->getDiagonalIndices(this->diagonalMatInd);

	/*x= new double[mat->dim()];
	b = new double[mat->dim()];*/

	for(int i = 0; i < mat->dim(); i++){
		b.push_back(0.0);
		x.push_back(0.0);
	}
	//solver->solve();
}

VectorFieldSolver::~VectorFieldSolver(void)
{
	delete solver;
	delete mat;
/*	delete[] x;
	delete[] b;*/
	delete l;
}

void VectorFieldSolver::solve(vector<int> & vertIDs, 
			vector<float> & src_sink_constr, 
			vector<int> & constr_edges,
			vector<tuple3f> & constr_edge_dir, 
			float edgeConstrWeight,
			float constrLength,
			VectorField * target )
{


	// * constrLength: Hack!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float weight = edgeConstrWeight * (Model::getModel()->getMeshInfo()->getHalfedges()->size());
	constraints(vertIDs, src_sink_constr, constr_edges, constr_edge_dir, weight * constrLength, &(b[0]));

	//want to store the oneform laplacian matrix M between two
	//calls, even Z changes.... this means it has to be tidied up 
	//after having added the extra constraints


	//TO DELETE:
	/*for(int i = 0; i < mat->a.size(); i++){
		mat->a[i] = 0;
	}
	for(int i = 0; i< diagonalMatInd.size(); i++){
		mat->a[diagonalMatInd[i]] = 1;
	}
	for(int i = 0; i < mat->dim(); i++){
		x[i]=0;
	}*/

	//LOOK OUT DELETE THE STUFF BEFORE HERE

mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_before.m");
mat->saveVector(b, "b", "C:/Users/bertholet/Dropbox/To Delete/b_constr.m" );

	l->addZToMat(constr_edges, diagonalMatInd, weight, mat);
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_wConstraints.m");

	delete solver;
	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat,1);
	solver->solve(&(x[0]),&(b[0]));
	l->substractZFromMat(constr_edges, diagonalMatInd, weight, mat);
	
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_after.m");

	for(int i = 0; i < mat->dim(); i++){
		target->setOneForm(i,1,(float) x[i]); //orientation = 1: solved for the edges as they are oriented.
	}
}

void VectorFieldSolver::constraints(vector<int> & vertIds, 
			vector<float> & src_sink_constr, 
			vector<int> & edgeIds,
			vector<tuple3f> & edge_dir_constr,
			float weight,
			double * b )
{
	for(int i = 0; i < mat->dim(); i++){
		b[i] = 0;
	}
	l->addZToB(edgeIds, edge_dir_constr,weight, b, mat->dim());

	l->add_star_d(vertIds, src_sink_constr, b, mat->dim());

}

void VectorFieldSolver::perturb( vector<int>& verts, vector<float> & src_sink_constr )
{
	l->perturb(verts, src_sink_constr);
}
