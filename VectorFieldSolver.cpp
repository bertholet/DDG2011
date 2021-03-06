#include "VectorFieldSolver.h"
#include "Model.h"
#include "vectorFieldTools.h"
#include <algorithm>

//#define PRINTMAT

VectorFieldSolver::VectorFieldSolver(mesh * aMesh, vector<tuple2i> & edges, vector<tuple3i> & f2he,
									 myStatusBar * statusBar)
{

	meshMetaInfo * msh = Model::getModel()->getMeshInfo();
	mat_border = new pardisoMatrix();
	mat_noborder = new pardisoMatrix();

	if(statusBar != NULL){
		statusBar->setBar(0,8);
	}

		
	*mat_border =	DDGMatrices::dual_d0(*msh) * DDGMatrices::star2(*msh) * DDGMatrices::d1(*msh);
	if(statusBar != NULL){
		statusBar->updateBar(1);
	}

	pardisoMatrix star0Inv = DDGMatrices::star0(*msh);
	if(statusBar != NULL){
		statusBar->updateBar(2);
	}
	pardisoMatrix star1 = DDGMatrices::star1(*msh);
	if(statusBar != NULL){
		statusBar->updateBar(3);
	}
	pardisoMatrix duald1_border = DDGMatrices::dual_d1(*msh);// + DDGMatrices::dual_d1_borderdiff(*mesh);
	if(statusBar != NULL){
		statusBar->updateBar(4);
	}

	*mat_noborder = *mat_border + (star1*pardisoMatrix::transpose(duald1_border) *star0Inv * (duald1_border) * star1);
	if(statusBar != NULL){
		statusBar->updateBar(5);
	}
	//comment for 'incorrect' border handling.
	duald1_border = duald1_border  + DDGMatrices::dual_d1_borderdiff(*msh);
	if(statusBar != NULL){
		statusBar->updateBar(6);
	}

	star0Inv.elementWiseInv(0);
	*mat_border = *mat_border + (star1*pardisoMatrix::transpose(duald1_border) *star0Inv * (duald1_border) * star1);
	if(statusBar != NULL){
		statusBar->updateBar(7);
	}

	//the following two lines DO work...
	l = new oneFormLaplacian(&f2he,&edges,aMesh);
	//mat = new pardisoMatrix();
	//mat->initMatrix(*l, edges.size(), statusBar);

	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

#ifdef PRINTMAT
/////////////////////////// ID IE IB IU IG ///////////////////////////////
/*
	meshMetaInfo & m = * Model::getModel()->getMeshInfo();
	pardisoMatrix d0 =  DDGMatrices::d0(m);
	pardisoMatrix d1 =  DDGMatrices::d1(m);
	pardisoMatrix delta1 =  DDGMatrices::delta1(m);
	pardisoMatrix delta2 =  DDGMatrices::delta2(m);
	pardisoMatrix star0 =  DDGMatrices::star0(m);
	pardisoMatrix star1 =  DDGMatrices::star1(m);
	pardisoMatrix star2 =  DDGMatrices::star2(m);
	pardisoMatrix mat2 = DDGMatrices::d0(m) * DDGMatrices::delta1(m) +
		DDGMatrices::delta2(m) * DDGMatrices::d1(m);
	//pardisoMatrix test = DDGMatrices::delta2(m) * DDGMatrices::d1(m);
	
	d0.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_d0.m");
	d1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_d1.m");
	delta1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_delta1.m");
	delta2.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_delta2.m");
	star0.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_star0.m");
	star1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_star1.m");
	star2.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_star2.m");
	mat2.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_ddglap.m");*/

	mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_before.m");

////////////////////////////////////////////////////////////////////////
#endif

	solver->setMatrix(*mat_border, 1);
	mat_border->getDiagonalIndices(this->diagonalMatInd);


	for(int i = 0; i < mat_border->dim(); i++){
		b.push_back(0.0);
		x.push_back(0.0);
	}

	if(statusBar != NULL){
		statusBar->updateBar(8);
	}
}

VectorFieldSolver::~VectorFieldSolver(void)
{
	delete solver;
	delete mat_border;
	delete mat_noborder;
	delete l;
}

void VectorFieldSolver::solve(vector<int> & vertIDs, 
			vector<float> & src_sink_constr, 
			vector<int> & constr_edges,
			vector<tuple3f> & constr_edge_dir, 
			float edgeConstrWeight,
			float constrLength,
			VectorField * target,
			bool borderAdapted)
{


	pardisoMatrix * mat = (borderAdapted? mat_border: mat_noborder);
	mat->getDiagonalIndices(this->diagonalMatInd);
	

	// * constrLength: Hack!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float weight = edgeConstrWeight * (Model::getModel()->getMeshInfo()->getHalfedges()->size());
	constraints(vertIDs, src_sink_constr, constr_edges, constr_edge_dir, weight * constrLength, &(b[0]));

	//want to store the oneform laplacian matrix M between two
	//calls, even Z changes.... this means it has to be tidied up 
	//after having added the extra constraints


#ifdef PRINTMAT
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_before.m");
mat->saveVector(b, "b", "C:/Users/bertholet/Dropbox/To Delete/b_constr.m" );
#endif

	l->addZToMat(constr_edges, diagonalMatInd, weight, mat);

#ifdef PRINTMAT
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_wConstraints.m");
#endif

	delete solver;
	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat,1);
	solver->solve(&(x[0]),&(b[0]));
	l->substractZFromMat(constr_edges, diagonalMatInd, weight, mat);

#ifdef PRINTMAT 
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_after.m");
#endif

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
	for(int i = 0; i < mat_border->dim(); i++){
		b[i] = 0;
	}
	l->addZToB(edgeIds, edge_dir_constr,weight, b, mat_border->dim());

	l->add_star_d(vertIds, src_sink_constr, b, mat_border->dim());

}

void VectorFieldSolver::constraints( vector<int> & vertIds, 
			vector<float> & src_sink_constr, 
			vector<int> & faceIds, 
			vector<tuple3f> & face_dir_constr, 
			vector<float> & lengths, 
			float weight, double * b )
{
	for(int i = 0; i < mat_border->dim(); i++){
		b[i] = 0;
	}

	l->addZToB(faceIds, face_dir_constr, lengths,weight, b, mat_border->dim());

	l->add_star_d(vertIds, src_sink_constr, b, mat_border->dim());

}




void VectorFieldSolver::constraintsSrcSinkOnly( vector<int> & vertIds, vector<float> & src_sink_constr, double * b )
{
	for(int i = 0; i < mat_border->dim(); i++){
		b[i] = 0;
	}
	l->add_star_d(vertIds, src_sink_constr, b, mat_border->dim());
}


void VectorFieldSolver::perturb( vector<int>& verts, vector<float> & src_sink_constr )
{
	l->perturb(verts, src_sink_constr);
}

void VectorFieldSolver::solveDirectional(vector<int> & vertIDs, 
										 vector<float> & src_sink_constr, 
										 vector<int> & constr_faces,
										 vector<tuple3f> & constr_face_dir, 
										 float edgeConstrWeight,
										 float constrLength,
										 VectorField * target,
										 bool borderAdapted)
{

	pardisoMatrix * mat = (borderAdapted? mat_border: mat_noborder);
	mat->getDiagonalIndices(this->diagonalMatInd);
	float weight = edgeConstrWeight;
	constraintsSrcSinkOnly(vertIDs, src_sink_constr, &(b[0]));


#ifdef PRINTMAT
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_before.m");
mat->saveVector(b, "b", "C:/Users/bertholet/Dropbox/To Delete/b_constr.m" );
#endif

	addDirConstraint2Mat(constr_faces, constr_face_dir, weight, mat);
	
	delete solver;
	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

#ifdef PRINTMAT
mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_wConstraints.m");
#endif

	solver->setMatrix(*mat,1);
	solver->solve(&(x[0]),&(b[0]));
	//subDirConstraint2Mat(constr_faces, constr_face_dir,  weight, mat);
	addDirConstraint2Mat(constr_faces, constr_face_dir, -weight, mat);

#ifdef PRINTMAT
	mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_after.m");
#endif

	for(int i = 0; i < mat->dim(); i++){
		target->setOneForm(i,1,(float) x[i]); //orientation = 1: solved for the edges as they are oriented.
	}
}

void VectorFieldSolver::addDirConstraint2Mat( vector<int> & constr_faces ,
											 vector<tuple3f> & constr_face_dir, 
											 float weight, 
											 pardisoMatrix * target )
{
	//float eps = 0.001;

	vector<tuple3i> & f2e = * Model::getModel()->getMeshInfo()->getFace2Halfedges();
	vector<tuple2i> & edges = * Model::getModel()->getMeshInfo()->getHalfedges();
	vector<tuple3i> & faces = Model::getModel()->getMesh()->getFaces();
	vector<tuple3f> & vert = Model::getModel()->getMesh()->getVertices();
	float e1,e2, e3, c1,c2,c3;
	int fcID;
	tuple3i fc, edgeIDs;
	tuple3f dir;

	for(int i = 0; i < constr_faces.size(); i++){
		fcID = constr_faces[i];
		fc = faces[fcID];
		edgeIDs = f2e[fcID];
		dir = constr_face_dir[i];

		e1= dir.dot(vert[fc.b]-vert[fc.a]) *fc.orientation(edges[edgeIDs.a]);
		e2= dir.dot(vert[fc.c]-vert[fc.b])*fc.orientation(edges[edgeIDs.b]);;
		e3= dir.dot(vert[fc.a]-vert[fc.c])*fc.orientation(edges[edgeIDs.c]);

		//the values under assumption of positive orientation
		/*c1 = e1/(e2+ eps);
		c2 = e2/(e3+ eps);
		c3 = e3/(e1+ eps);*/
		c1 = e1/(e2);
		c2 = e2/(e3);
		c3 = e3/(e1);

		target->add(edgeIDs.a,edgeIDs.a, weight* (1+c3*c3));
		target->add(edgeIDs.a,edgeIDs.b, -weight * c1);
		target->add(edgeIDs.a,edgeIDs.c, -weight * c3);

		target->add(edgeIDs.b,edgeIDs.a, -weight* c1);
		target->add(edgeIDs.b,edgeIDs.b, weight * (1+c1*c1));
		target->add(edgeIDs.b,edgeIDs.c, -weight * c2);

		target->add(edgeIDs.c,edgeIDs.a, -weight* c3);
		target->add(edgeIDs.c,edgeIDs.b, -weight * c2);
		target->add(edgeIDs.c,edgeIDs.c, weight * (1+c2*c2));
	}
}

void VectorFieldSolver::solveLengthEstimated( vector<int> & vertIDs, 
			vector<float> & src_sink_constraints, 
			vector<int> & constr_fc,
			vector<tuple3f> & constr_fc_dir, 
			float weight, VectorField * target, bool borderAdapted )
{

	pardisoMatrix * mat = (borderAdapted? mat_border: mat_noborder);
	mat->getDiagonalIndices(this->diagonalMatInd);

	vector<float> length;
	vector<int> constr_edges;

	findLengths(vertIDs, src_sink_constraints, constr_fc,
		length, mat);

	constraints(vertIDs, src_sink_constraints, 
		constr_fc, constr_fc_dir, length, weight, &(b[0]));

	pushEdges(constr_fc, * Model::getModel()->getMeshInfo()->getFace2Halfedges(), constr_edges);

#ifdef PRINTMAT
	mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_before.m");
	mat->saveVector(b, "b", "C:/Users/bertholet/Dropbox/To Delete/b_constr.m" );
#endif

	l->addZToMat(constr_edges, diagonalMatInd, weight, mat);

#ifdef PRINTMAT
	mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_wConstraints.m");
#endif

	delete solver;
	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat,1);
	solver->solve(&(x[0]),&(b[0]));
	l->substractZFromMat(constr_edges, diagonalMatInd, weight, mat);

#ifdef PRINTMAT 
	mat->saveMatrix("C:/Users/bertholet/Dropbox/To Delete/matrix_after.m");
#endif

	for(int i = 0; i < mat->dim(); i++){
		target->setOneForm(i,1,(float) x[i]); //orientation = 1: solved for the edges as they are oriented.
	}

}

void VectorFieldSolver::findLengths( vector<int> & vertIDs, vector<float> & src_sink_constr, 
				vector<int> & constr_fc, 
				vector<float> & target_lengths,
				pardisoMatrix * mat)
{

	target_lengths.clear();

	constraintsSrcSinkOnly(vertIDs,src_sink_constr, &(b[0]));

	delete solver;
	solver = new pardisoSolver(pardisoSolver::MT_STRUCTURALLY_SYMMETRIC,
		pardisoSolver::SOLVER_ITERATIVE, 3);

	solver->setMatrix(*mat,1);
	solver->solve(&(x[0]),&(b[0]));

	tuple3i f2edgs;
	tuple3i fc;
	tuple3f oneform;
	tuple3f baricoords(1.f/3,11.f/3,1.f/3);
	tuple3f target_vec;
	vector<tuple2i> & edgs = * Model::getModel()->getMeshInfo()->getHalfedges();
	vector<tuple3i> & f2e = * Model::getModel()->getMeshInfo()->getFace2Halfedges();
	vector<tuple3i> & fcs = Model::getModel()->getMesh()->getFaces();
	vector<tuple3f> & verts = Model::getModel()->getMesh()->getVertices();

	for(int i = 0; i < constr_fc.size(); i++){
		fc = fcs[constr_fc[i]];
		f2edgs = f2e[constr_fc[i]];
		oneform.x = x[f2edgs.a];
		oneform.y = x[f2edgs.b];
		oneform.z = x[f2edgs.c];
		vectorFieldTools::oneFormToVector(fc, f2edgs, edgs, verts, oneform,baricoords,target_vec);

		target_lengths.push_back(target_vec.norm());

	}


}

void VectorFieldSolver::pushEdges( vector<int> & constr_fc, vector<tuple3i> & f2e , vector<int> & target )
{
	int edg;
	for(int i = 0; i < constr_fc.size(); i++){
		edg = f2e[constr_fc[i]].a;
		if(find(target.begin(),target.end(), edg) == target.end()){
			target.push_back(edg);
		}
		edg = f2e[constr_fc[i]].b;
		if(find(target.begin(),target.end(), edg) == target.end()){
			target.push_back(edg);
		}
		edg = f2e[constr_fc[i]].c;
		if(find(target.begin(),target.end(), edg) == target.end()){
			target.push_back(edg);
		}
	}
}


