#include "fluidSimulation.h"
#include "fluidTools.h"
#include <math.h>
#include "meshOperation.h"
#include "dualMeshTools.h"
#include "Model.h"
#include <limits>
#include "matrixf.h"
#include "matrixFactory.h"
#include "stdafx.h"
#include "pardiso.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <iostream>
#include <omp.h>


//#define DISPLAY_VORTS
//#define printMat
//#define printMat_harmonicFlux
#define VARIANT_ONE

fluidSimulation::fluidSimulation( meshMetaInfo * mesh ):
flux(*mesh),harmonicFlux(*mesh), vorticity(*mesh), L_m1Vorticity(*mesh), tempNullForm(*mesh), forceFlux(*mesh)
{
	myMesh = mesh;
	simulationtime = 0;

	dualMeshTools::getDualVertices(*mesh, dualVertices);
//	assert(checkAllDualVerticesInside());

	backtracedDualVertices = dualVertices; //this copies everything.
	backtracedVelocity = dualVertices; // just to have the right dimension
	backtracedVelocity_noHarmonic = dualVertices; // same reason



	//for visualisation
	line_strip_triangle.reserve(dualVertices.size());
	line_stripe_starts.reserve(dualVertices.size()); // for visualisation
	age.reserve(dualVertices.size());
	maxAge = 200;
	int noFaces = myMesh->getBasicMesh().getFaces().size();
	srand(0);

	for(int j = 0; j < 100/*noFaces*/; j++){

		int i = (rand()%noFaces);
		line_strip_triangle.push_back(i);//(rand()%noFaces);
		line_stripe_starts.push_back(/**/randPoint(i));//*/dualVertices[line_strip_triangle[i]]);
		age.push_back(rand()%maxAge);
	}


	//for Backtracing
	triangle_btVel.reserve(dualVertices.size());
	velocities.reserve(dualVertices.size());
	harmonicVelocities.reserve(dualVertices.size());
	for(int i = 0; i < dualVertices.size(); i++){
		triangle_btVel.push_back(-1);
		velocities.push_back(tuple3f());
		harmonicVelocities.push_back(tuple3f());
		//velocitie.push_back(tuple3f());
	}


	initVertexOnBorder();


	//matrix set up.
	addDiffusionSolver = NULL;
	vort2FluxSolver = NULL;
	setupMatrices();

	this->setStepSize(0);
	this->setViscosity(0);

	//for display
	maxVort = 1;
	minVort = 0;
	showStreamLines = false;
	streamlineLength = 5;
	doInterpolation = true;
	showVortNotSpeed = true;

//////////////////////////////////////////////////////////////////////////
	//make sure everything is init for parallel loop..
	myMesh->getBorder();
	myMesh->getCurvNormals();
	myMesh->getFace2Halfedges();
	myMesh->getHalfedges();

//#ifdef printMat
//	L.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/laplace0.m");
//#endif

}

fluidSimulation::~fluidSimulation(void)
{
	if(addDiffusionSolver != NULL){
		delete addDiffusionSolver;
	}

	if(vort2FluxSolver != NULL){
		delete vort2FluxSolver;
	}
}


void fluidSimulation::initVertexOnBorder()
{
	vertexOnBorder.clear();
	vertexOnBorder.resize(myMesh->getBasicMesh().getVertices().size(),false);
	for(int i = 0; i < myMesh->getBorder().size(); i++){
		std::vector<int> & brdr = myMesh->getBorder()[i];
		for(int j = 0; j < brdr.size(); j++){
			vertexOnBorder[brdr[j]]=true;
		}
	}
}


void fluidSimulation::setupMatrices()
{

	//matrix setup for 0 viscosity.
	bool zeroViscosity = false;

	d0 =DDGMatrices::d0(*myMesh);
	//borderdiff is zero if there is no border.
	//	dt_star1 = (DDGMatrices::dual_d1(*myMesh) +DDGMatrices::dual_d1_borderdiff(*myMesh))* DDGMatrices::star1(*myMesh);
	dt_star1 = ((DDGMatrices::dual_d1(*myMesh))* DDGMatrices::star1(*myMesh));// + DDGMatrices::dual_d1star1_borderdiff(*myMesh);
	if(zeroViscosity){
		dt_star1 = dt_star1 + DDGMatrices::dual_d1star1_borderdiff(*myMesh);
	}

//	pardisoMatrix border = DDGMatrices::onesBorder(myMesh->getBorder(),L.getn(), L.getn());
//	border *= 1000;

	
		L = (dt_star1 + (DDGMatrices::dual_d1(*myMesh))* 
			DDGMatrices::onesBorderEdges(*myMesh)* 100000)*
			d0;
	
		if(vort2FluxSolver != NULL){
			delete vort2FluxSolver;
		}
		vort2FluxSolver = new pardisoSolver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,5);
		vort2FluxSolver->setMatrix(L,1);

//	L = dt_star1*d0;/*DDGMatrices::dual_d1(*myMesh) * DDGMatrices::star1(*myMesh) * d0;*/
	//add stuff for 0 on border condition. Might be a bad idea.
//	L = L + border;

	//trying variant which should be correct for the backtransformation

	star0 = DDGMatrices::star0(*myMesh);
}


//////////////////////////////////////////////////////////////////////////
// Getters and Setters
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// sets the viscosity and adapts the star_minus_vhL matrix.
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::setViscosity( float visc )
{
	viscosity = visc;
	//calc vhL;
	/*pardisoMatrix dualD1 = DDGMatrices::dual_d1()
	pardisoMatrix zeroVortDt_star1;*/

	star0_min_vhl = (dt_star1)*d0; //was = L (i.e. with constriction of zero flux on borders) 
	star0_min_vhl *= viscosity*timeStep;

#ifdef printMat
	star0_min_vhl.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vtL0.m");
#endif

	//the final matrix
	star0_min_vhl = star0 - star0_min_vhl; //was star0 not id0. WAS MINUS !!! Now IS minus because of star1 assumption

	if(addDiffusionSolver != NULL){
		delete addDiffusionSolver;
	}
	addDiffusionSolver = new pardisoSolver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,5);
	addDiffusionSolver->setMatrix(star0_min_vhl,1);

#ifdef printMat
	star0_min_vhl.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/id0_min_vtL0.m");
#endif
}

//////////////////////////////////////////////////////////////////////////
// Set the size of the timestep
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::setStepSize( float stepSize )
{
	this->timeStep = stepSize;
}

//////////////////////////////////////////////////////////////////////////
// sets the flux exactly to the provided oneform
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::setFlux( oneForm & f )
{
	assert(f.getMesh() == myMesh);
	flux = f;

	updateVelocities();
}

//////////////////////////////////////////////////////////////////////////
// converts the directions provided to a flux and sets this flux. Note
// that the directions will change, manly due to the fact that only
// incompressible fluxes can be treated. The directions provided
// will usually contradict the incompressibility.
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::setFlux( vector<tuple3f> & dirs )
{
	fluidTools::dirs2Flux(dirs,flux,*myMesh/*, dualVertices*/);
}

//////////////////////////////////////////////////////////////////////////
// converts the directions tu flux and uses this flux as force (e.g. stirring)
// the same restriction as for setFlux hold.
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::setForce(vector<tuple3f> & dirs)
{
	fluidTools::dirs2Flux(dirs,forceFlux,*myMesh/*, dualVertices*/);
}

#ifdef VARIANT_ONE
oneForm fluidSimulation::setHarmonicFlow( vector<tuple3f> & borderConstraints )
{
	//benutzt eigene Matrizen weil unabhängig von viscosität frage.
	harmonicFlux.setZero();
	if(myMesh->getBorder().size() == 0){
		//only for bordered meshs.
		return harmonicFlux;
	}


	// setting flux constraint Weight
	float weight = 10;
	
	//mesh properties
	vector<vector<int>> & brdr = myMesh->getBorder();
	vector<tuple2i> & edgs = * myMesh->getHalfedges();
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	
	//Matrix setUp
	
	//compute a border adapted least square formulation laplacian.
	pardisoMatrix d1 = DDGMatrices::d1(*myMesh);
	pardisoMatrix star0inv = DDGMatrices::star0(*myMesh);
	star0inv.elementWiseInv(0);
	//adapting duald1 to consider inner border(s).
	pardisoMatrix duald1 = DDGMatrices::dual_d1(*myMesh);
	/*{
		pardisoMatrix borderVerticesT;
		vector<double> vals;
		for(int i = 1; i < brdr.size(); i++){
			vector<int> indices = brdr[i];
			sort(indices.begin(), indices.end());

			vals.reserve(brdr[i].size());
			for(int j = 0; j < brdr[i].size(); j++){
				vals.push_back(1);
			}
			borderVerticesT.addLine(indices, vals);
			borderVerticesT.forceNrColumns(myMesh->getBasicMesh().getVertices().size());

			(borderVerticesT * duald1).getLine(0,indices,vals);
			duald1.addLine(indices,vals);
		}
		
	}*/

	adaptMatrices_zeroTotalBorderVort(brdr, star0inv, duald1);
	
	pardisoMatrix dt_star1_adapted = duald1 * DDGMatrices::star1(*myMesh) ;//+ DDGMatrices::dual_d1star1_borderdiff(*myMesh);
	
	//oneForm constFlow(*myMesh);//, harmonicFlux(*myMesh);
	vector<double> buff;
	vector<double> fluxConstr = harmonicFlux.getVals(); //init to zero, right size.
	int sz,edgeId;
	tuple2i edge;


	pardisoMatrix Lflux = (pardisoMatrix::transpose(d1)*DDGMatrices::star2(*myMesh)*d1) //* 100
		+ pardisoMatrix::transpose(dt_star1_adapted)*star0inv*dt_star1_adapted;

#ifdef printMat_harmonicFlux
	std:vector<int> borderEdges;
	borderEdges.reserve(edgs.size());
	Lflux.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/LaplaceFlux_unconstrained.m");
#endif

	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
//		constFlow.initToConstFlux(borderConstraints[i]);
//		dual1.mult(constFlow.getVals(), buff, true);

		//buff is now the vorticites on dual faces

		//Lflux.mult(constFlow.getVals(), buff,true);
		for(int j = 0; j < sz;j++){
			edgeId =myMesh->getHalfedgeId(brdr[i][j%sz], brdr[i][(j+1)%sz],&edge);
			assert(edgeId >=0);
			assert((edgs[edgeId].a == brdr[i][j%sz] && edgs[edgeId].b == brdr[i][(j+1)%sz] )||
				(edgs[edgeId].b == brdr[i][j%sz] && edgs[edgeId].a == brdr[i][(j+1)%sz]));

			Lflux.add(edgeId,edgeId,weight);
			fluxConstr[edgeId] = borderConstraints[i].dot(verts[edge.b] -verts[edge.a]) * weight;// + buff[edgeId];

#ifdef printMat_harmonicFlux
			borderEdges.push_back(edgeId);;
#endif

		}
	}



	//solve for the harmonic flux.
	{pardisoSolver solver(pardisoSolver::MT_ANY, pardisoSolver::SOLVER_DIRECT,15);
	solver.setMatrix(Lflux,1);
	solver.setStoreResultInB(false);
	solver.solve(& (harmonicFlux.getVals()[0]), & (fluxConstr[0]));
	}
	//already stored in harmonicFlux.

	
	//substract vorticity field from harmonic field.
/*	fluidTools::flux2Vorticity(harmonicFlux,vorticity,*myMesh,dt_star1);
	oneForm vortPart(*myMesh);
	vorticity2Flux(vorticity,vortPart);
	harmonicFlux.add(vortPart,-1);*/


	//UPDATE STUFF
	fluidTools::flux2Vorticity(harmonicFlux,vorticity,*myMesh,dt_star1);
	fluidTools::flux2Velocity(harmonicFlux,harmonicVelocities, *myMesh);
	
	
	updateVelocities();


#ifdef printMat_harmonicFlux
	for(int i = 0; i < fluxConstr.size(); i++){
		fluxConstr[i] /= weight;
	}
	Lflux.saveVector(fluxConstr, "flux_constraint","C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/fluxConstr.m");
	Lflux.saveVector(borderEdges, "border_edges","C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/borderEdges.m");
	Lflux.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/LaplaceFlux.m");
#endif // printMat_harmonicFlux

	return harmonicFlux;

}
#endif // VARIANT_ONE

#ifndef VARIANT_ONE
//this is a working implementation, where the vorticity issue around inner 
// boundaries is solved by substracting the vorticity induced field  from
// the harmonic solution
oneForm fluidSimulation::setHarmonicFlow( vector<tuple3f> & borderConstraints )
{
	//benutzt eigene Matrizen weil unabhängig von viscosität frage.
	harmonicFlux.setZero();
	if(myMesh->getBorder().size() == 0){
		//only for bordered meshs.
		return harmonicFlux;
	}
	//compute a border adapted least square formulation laplacian.
	pardisoMatrix d1 = DDGMatrices::d1(*myMesh);
	pardisoMatrix star0inv = DDGMatrices::star0(*myMesh);
	star0inv.elementWiseInv(0);
	pardisoMatrix dt_star1_adapted = DDGMatrices::dual_d1(*myMesh) * DDGMatrices::star1(*myMesh) ;//+ DDGMatrices::dual_d1star1_borderdiff(*myMesh);

	//	pardisoMatrix Lflux = (pardisoMatrix::transpose(d1)*DDGMatrices::star2(*myMesh)*d1) //* 100
//		+ pardisoMatrix::transpose(dt_star1)*star0inv*dt_star1;

	// setting flux constraints
	float weight = 10;
	
	vector<vector<int>> & brdr = myMesh->getBorder();
	vector<tuple2i> & edgs = * myMesh->getHalfedges();
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	oneForm constFlow(*myMesh);//, harmonicFlux(*myMesh);
	vector<double> buff;
	vector<double> fluxConstr = harmonicFlux.getVals(); //init to zero, right size.
	int sz,edgeId;
	tuple2i edge;

//adapt matrix....
	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
		for(int j = 0; j < sz;j++){
			//'unconstrained'
			star0inv.setLineToZero(brdr[i][j]);
		}
	}


	pardisoMatrix Lflux = (pardisoMatrix::transpose(d1)*DDGMatrices::star2(*myMesh)*d1) //* 100
		+ pardisoMatrix::transpose(dt_star1_adapted)*star0inv*dt_star1_adapted;

#ifdef printMat_harmonicFlux
	std:vector<int> borderEdges;
	borderEdges.reserve(edgs.size());
	Lflux.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/LaplaceFlux_unconstrained.m");
#endif

	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
//		constFlow.initToConstFlux(borderConstraints[i]);
//		dual1.mult(constFlow.getVals(), buff, true);

		//buff is now the vorticites on dual faces

		//Lflux.mult(constFlow.getVals(), buff,true);
		for(int j = 0; j < sz;j++){
			edgeId =myMesh->getHalfedgeId(brdr[i][j%sz], brdr[i][(j+1)%sz],&edge);
			assert(edgeId >=0);
			assert((edgs[edgeId].a == brdr[i][j%sz] && edgs[edgeId].b == brdr[i][(j+1)%sz] )||
				(edgs[edgeId].b == brdr[i][j%sz] && edgs[edgeId].a == brdr[i][(j+1)%sz]));

			Lflux.add(edgeId,edgeId,weight);
			fluxConstr[edgeId] = borderConstraints[i].dot(verts[edge.b] -verts[edge.a]) * weight;// + buff[edgeId];

#ifdef printMat_harmonicFlux
			borderEdges.push_back(edgeId);;
#endif

		}
	}



	//solve for the harmonic flux.
	{pardisoSolver solver(pardisoSolver::MT_ANY, pardisoSolver::SOLVER_DIRECT,15);
	solver.setMatrix(Lflux,1);
	solver.setStoreResultInB(false);
	solver.solve(& (harmonicFlux.getVals()[0]), & (fluxConstr[0]));
	}
	//already stored in harmonicFlux.

	
	//substract vorticity field from harmonic field.
	fluidTools::flux2Vorticity(harmonicFlux,vorticity,*myMesh,dt_star1);
	oneForm vortPart(*myMesh);
	vorticity2Flux(vorticity,vortPart);
	harmonicFlux.add(vortPart,-1);


	//UPDATE STUFF
	fluidTools::flux2Vorticity(harmonicFlux,vorticity,*myMesh,dt_star1);
	fluidTools::flux2Velocity(harmonicFlux,harmonicVelocities, *myMesh);
	updateVelocities();

#ifdef printMat_harmonicFlux
	for(int i = 0; i < fluxConstr.size(); i++){
		fluxConstr[i] /= weight;
	}
	Lflux.saveVector(fluxConstr, "flux_constraint","C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/fluxConstr.m");
	Lflux.saveVector(borderEdges, "border_edges","C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/borderEdges.m");
	Lflux.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/LaplaceFlux.m");
#endif // printMat_harmonicFlux

	return harmonicFlux;

}
#endif //VARIANT_ONE



oneForm & fluidSimulation::getHarmonicFlux()
{
	return this->harmonicFlux;
}


//////////////////////////////////////////////////////////////////////////
// get the Flux oneForm
//////////////////////////////////////////////////////////////////////////
oneForm & fluidSimulation::getFlux()
{
	return flux;
}


//////////////////////////////////////////////////////////////////////////
// (((The Actual Algorithm)))
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Do one Timestep. Does everything
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::oneStep()
{
	cout << "time since last step: " << timer_in_between.elapsed();
	timer.restart();
	timer_total.restart();

	pathTraceDualVertices(timeStep); 

	cout << "pathTracing" << timer.elapsed() << "\n";
	timer.restart();

	updateBacktracedVelocities(); //is ok if isOnoutside border not denoted

	cout << "velocity interpolation" << timer.elapsed() << "\n";
	timer.restart();


	backtracedVorticity(); //should be fine

	cout << "vorticity computation" << timer.elapsed() << "\n";
	timer.restart();

	addForces2Vorticity(timeStep); 

	cout << "add forces" << timer.elapsed() << "\n";
	timer.restart();

	addDiffusion2Vorticity();

	cout << "adding diffusion" << timer.elapsed() << "\n";
	timer.restart();

	vorticity2Flux();

	cout << "vort 2 flux" << timer.elapsed() << "\n";
	timer.restart();

	cout << "Total: " << timer_total.elapsed();

//testFlux();

	timer_in_between.restart();
	updateVelocities();
	simulationtime += timeStep;

	Model::getModel()->setVectors(&dualVertices,&velocities, false);
}

void fluidSimulation::walkPath( tuple3f * pos, int * triangle, float * t, bool * hitBorder, 
				std::vector<float> & weight_buffer, int direction )
{
	tuple3f dir;
	//velocity is stored in dir:
	getVelocityFlattened(*pos,*triangle, dir, weight_buffer);
	dir*=direction;
	tuple3f cut_pos;
	tuple2i cut_edge;
	float max_t = maxt(*pos, *triangle, dir, cut_pos, cut_edge);


	if(*t <= max_t ){
		*pos = *pos + dir * (*t);
		*t = 0;
	}
	else{
		//leaving the triangle.
		pos->set(cut_pos);
		*t = *t - max_t;
		int temp = meshOperation::getNegFace(cut_edge, myMesh->getBasicMesh());
		assert(temp!= *triangle);

		//if temp < 0 isOutside = true;
		if(temp < 0 ){
			*hitBorder = true;
		}
		else{
			*triangle = temp; 
			*hitBorder = false;
		}
	}
}


float fluidSimulation::maxt( tuple3f & pos, int triangle, tuple3f & dir, tuple3f & cutpos, tuple2i & edge )
{
	assert(dir.x == dir.x && dir.y == dir.y && dir.z == dir.z);
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	tuple3i& tr = myMesh->getBasicMesh().getFaces()[triangle];
	tuple3f & a = verts[tr.a];
	tuple3f & b = verts[tr.b];
	tuple3f & c = verts[tr.c];


	tuple3f n = (b-a).cross(c-a);
	n.normalize();
	//normals pointing into the triangle
/*	tuple3f n1 = (a-b).cross(n);
	tuple3f n2 = (b-c).cross(n);
	tuple3f n3 = (c-a).cross(n);*/

	//normals on the sides.

//	n1.normalize();
//	n2.normalize();
//	n3.normalize();
	float max_t = numeric_limits<float>::infinity();
	
	tuple3f n1 = (a-b).cross(n);
	float t= n1.dot(a-pos)/n1.dot(dir);
	if(t==t && t>=-0.00001 && dir.dot(n1)<0){
		max_t = t;
		edge.set(tr.a,tr.b);
	}

	n1 = (b-c).cross(n);
	t = n1.dot(b-pos)/n1.dot(dir);
	if(t==t && t>= -0.00001 && t<max_t && dir.dot(n1)<0){
		max_t = t;
		edge.set(tr.b, tr.c);
	}

	n1 = (c-a).cross(n);
	t = n1.dot(c-pos)/n1.dot(dir);
	if(t==t && t>= -0.00001 && t<max_t && dir.dot(n1)<0){
		max_t = t;
		edge.set(tr.c, tr.a);
	}
	//float t2 = n2.dot(b-pos)/n2.dot(dir);
	/*if(t2==t2 && t2>= -0.00001 && t2<max_t && dir.dot(n2)<0){
		max_t = t2;
		edge.set(tr.b, tr.c);
	}
	float t3 = n3.dot(c-pos)/n3.dot(dir);
	if(t3==t3 && t3>= -0.00001 && t3<max_t && dir.dot(n3)<0){
		max_t = t3;
		edge.set(tr.c, tr.a);
	}*/
	
//	assert(max_t == max_t && max_t!=numeric_limits<float>::infinity() &&max_t>=0);

	cutpos.set(pos+dir*max_t);
/*	if(t==max_t){
		edge.set(tr.a,tr.b);
	}
	else if(t2==max_t){
		edge.set(tr.b, tr.c);
	}
	else if(t3==max_t){
		edge.set(tr.c, tr.a);
	}*/

	return max_t;
}


void fluidSimulation::pathTraceDualVertices( float t )
{

	//std::vector<tuple3i> & fcs = myMesh->getBasicMesh().getFaces();
	backtracedDualVertices = dualVertices;
	int triangle;
	int nrIterations = t/0.05 + 1; // at least one iteration!!!
	int end = backtracedDualVertices.size();
	float changed_t;
	bool hitBorder;
	std::vector<float> intern_memory; //intern memory
	
#pragma omp parallel for private(triangle, changed_t, intern_memory, hitBorder) num_threads(8)
	for(int i = 0; i < end; i++){
		//cout <<"bazinga! thread " << omp_get_thread_num();
		triangle = i;
		hitBorder = false;
		for(int j = 0; j < nrIterations;j++){
			changed_t = t/nrIterations;
			//triangle = -1 => outside of mesh
			while(changed_t > 0.0000001 && !hitBorder/*triangle!=-1*/){

				walkPath(&(backtracedDualVertices[i]), &triangle,&changed_t, &hitBorder,intern_memory);
			}
		}
		triangle_btVel[i]=triangle;
	}
}



void fluidSimulation::vorticity2Flux()
{
	vorticity2Flux(vorticity,flux);
	/*	pardisoSolver solver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,3);
	solver.setMatrix(L,1);
//	star0_inv.mult((vorticity.getVals()),(tempNullForm.getVals()));
	assert(vorticity.getVals()[vorticity.getVals().size()/2] < 10E10 &&
		vorticity.getVals()[vorticity.getVals().size()/2] > -10E10 );


	//zero border condition... thats stupid, the vorticity is not 0: the flux should be.
	/*for(int i = 0; i < myMesh->getBorder().size(); i++){
		vector<int> & brdr = myMesh->getBorder()[i];
		for(int j = 0; j < brdr.size(); j++){
			vorticity.getVals()[brdr[j]] = 0;
		}
	}*/

/*	solver.solve(&(L_m1Vorticity.getVals()[0]), & (vorticity.getVals()[0]));
	d0.mult(L_m1Vorticity.getVals(),flux.getVals());
	assert(flux.getVals()[flux.getVals().size()/2] < 10E10 &&
		flux.getVals()[flux.getVals().size()/2] > -10E10 );*/
}

void fluidSimulation::vorticity2Flux( nullForm & vort, oneForm & target )
{
//	pardisoSolver solver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,3);
//	solver.setMatrix(L,1);

	assert(vort.getVals()[vort.getVals().size()/2] < 10E10 &&
		vort.getVals()[vort.getVals().size()/2] > -10E10 );



//	solver.solve(&(L_m1Vorticity.getVals()[0]), & (vort.getVals()[0]));
	vort2FluxSolver->solve(&(L_m1Vorticity.getVals()[0]), & (vort.getVals()[0]));

	d0.mult(L_m1Vorticity.getVals(),target.getVals());
	assert(target.getVals()[target.getVals().size()/2] < 10E10 &&
		target.getVals()[target.getVals().size()/2] > -10E10 );


}


void fluidSimulation::flux2Vorticity()
{
	fluidTools::flux2Vorticity(flux,vorticity, *myMesh, dt_star1);
}





void fluidSimulation::getVelocityFlattened( tuple3f & pos, int actualTriangle, tuple3f & result, 
					std::vector<float> & weights,bool useHarmonicField)
{

	result.set(0,0,0);
	if(actualTriangle <0){
		return;
		//return tuple3f();
	}

	if(!doInterpolation){
		result.set(velocities[actualTriangle]);
		return;
	}
	
	assert(actualTriangle >=0);
	std::vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	tuple3i & tr = myMesh->getBasicMesh().getFaces()[actualTriangle];
	//determine actual dual Face 22 ms per go up to here.
	int dualFace;
	float d1 = (verts[tr.a]-pos).norm();
	float d2 = (verts[tr.b]-pos).norm();
	float d3 = (verts[tr.c]-pos).norm();
	float mn = min(min(d1,d2),d3);
	if(mn == d1){
		dualFace = tr.a;
	}
	else if(mn == d2){
		dualFace = tr.b;
	}
	else if ( mn == d3){
		dualFace = tr.c;
	}
	else{
		// NAN....
		assert(false);
	}


	//determine weights;
	fluidTools::bariCoords(pos,dualFace,dualVertices, weights, *myMesh);

	// dualVertices of dualFace;
	std::vector<int> & dualVertIDs = myMesh->getBasicMesh().getNeighborFaces()[dualFace];

	if(myMesh->getBorder().size() != 0){
		//on bordered meshs the stuff with the curv normal does not work.
		//for now assum that meshes are flat if they have a border
		assert(weights.size() == dualVertIDs.size());
		for(int i = 0; i < weights.size(); i++){
			assert(weights[i]== weights[i]);
			result += (velocities[dualVertIDs[i]])*weights[i];
			if(!useHarmonicField){
				result -= harmonicVelocities[dualVertIDs[i]]*weights[i];
			}
		}
	}
	else{
		tuple3f curvNormal = (*myMesh->getCurvNormals())[dualFace];
		curvNormal.normalize();
		tuple3f triangleNormal = (verts[tr.b]-verts[tr.a]).cross(verts[tr.c]-verts[tr.a]);
		tuple3f vel;

		for(int i = 0; i < weights.size(); i++){
			assert(weights[i]== weights[i]);
			vel.set(velocities[dualVertIDs[i]]);
			//vel+=harmonicVelocities[dualVertIDs[i]]; done at update velocity step.
			//result += turnVelocityInRightPlane(velocities[dualVertIDs[i]], dualVertIDs[i], actualTriangle)*weights[i];
			//result += (velocities[dualVertIDs[i]] - curvNormal * (curvNormal.dot(velocities[dualVertIDs[i]])) )*weights[i];
			result += (vel - curvNormal * (curvNormal.dot(vel) ))*weights[i];
		}

		result = result - curvNormal * (result.dot(triangleNormal) / curvNormal.dot(triangleNormal));
			assert(result.x == result.x && result.y == result.y && result.z == result.z);
	}
	assert(result.x *0 == 0 && result.y*0 ==0 && result.z*0 == 0);
	//return result;
}


//////////////////////////////////////////////////////////////////////////
//Bugs:
//maybe overly imprecise on curved surfaces because of taking euclidean distance
// which is a CRUDE approx of the geodesic distance (which is implicitely
// used in the dual edge edge ratios)
// make sure backtraced velocities were updated.
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::backtracedVorticity()
{
	std::vector<std::vector<int>> & dualf2v = myMesh->getBasicMesh().getNeighborFaces();
	std::vector<double> & vort = vorticity.getVals();
	bool anyVertexOutside, useHarmonicField;

	//updateBacktracedVelocities();
	double temp;
	int start,sz, stop;
#pragma omp parallel for private(temp,sz,start,stop,anyVertexOutside,useHarmonicField) num_threads(8)
	for(int i = 0; i < vorticity.size();i++){ // < nrVerts.size
		temp = 0;
		anyVertexOutside= false;
		useHarmonicField = false;

		//the dual vertices surrounding the face.
		std::vector<int> & dualV = dualf2v[i];
		sz = dualV.size();
		start = 0;
		stop = sz;
		if(viscosity != 0 && vertexOnBorder[i]){
			//do not take the flow along border edge into account.
			stop = sz-1;
			useHarmonicField = true;
		}
		for(int j = start; j < stop; j++){

			if(triangle_btVel[dualV[j]] == -1){
				anyVertexOutside = true;
//				continue; // velocity is assumed to be zero on this edge
			}
			// was += now -= because the dual edges are oriented in the oposite way of following the border of the
			// one ring of vertex.!!!
			if(useHarmonicField){
				temp -= 0.5* ((backtracedVelocity[dualV[j]] + backtracedVelocity[dualV[(j+1)%sz]]).dot(
					backtracedDualVertices[dualV[(j+1)%sz]] - backtracedDualVertices[dualV[j]])); 
			}
			else{
				temp -= 0.5* ((backtracedVelocity_noHarmonic[dualV[j]] + backtracedVelocity_noHarmonic[dualV[(j+1)%sz]]).dot(
					backtracedDualVertices[dualV[(j+1)%sz]] - backtracedDualVertices[dualV[j]])); 
			}
		}
		vort[i] =temp;


		/*if(!anyVertexOutside){
			vort[i] = 0;
		}*/
	}
}

void fluidSimulation::updateBacktracedVelocities()
{
	//alloc internal memory
	std::vector<float> intern_mem;
	intern_mem.reserve(20);

#pragma omp parallel for private(intern_mem) num_threads(8)
	for(int i = 0; i < backtracedVelocity.size(); i++){
		//store velocity in backTracedVelocity[i] =...
		getVelocityFlattened(backtracedDualVertices[i],triangle_btVel[i],backtracedVelocity[i], intern_mem, true); 
		getVelocityFlattened(backtracedDualVertices[i],triangle_btVel[i],backtracedVelocity_noHarmonic[i], intern_mem, false); 
	}

	//trying something
	/*vector<vector<int>> & v2f = myMesh->getBasicMesh().getNeighborFaces();
	vector<vector<int>> & brdr = myMesh->getBorder();
	for(int i = 0; i < brdr.size(); i++){
		for(int j = 0; j < brdr[i].size(); j++){
			vector<int> & nbrfaces = v2f[brdr[i][j]];
			for(int k = 0; k < nbrfaces.size(); k++){
				backtracedVelocity[nbrfaces[k]].set(0,0,0);
			}
		}
	}*/
}






void fluidSimulation::updateVelocities()
{
	fluidTools::flux2Velocity(flux,velocities, *myMesh);

	for(int i = 0; i < velocities.size(); i++){
		velocities[i]+= harmonicVelocities[i];
		assert(velocities[i].x *0 == 0);
	}
}


void fluidSimulation::addForces2Vorticity(float timeStep)
{
	dt_star1.mult(forceFlux.getVals(),tempNullForm.getVals());
	vorticity.add(tempNullForm, timeStep);
}


void fluidSimulation::addDiffusion2Vorticity()
{
//	pardisoSolver solver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,3);
//	solver.setMatrix(star0_min_vhl,1);


#ifdef printMat
	L.saveVector(vorticity.getVals(),"vort_before","C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vort_before_diffusion.m");
#endif
//	solver.solve(&(tempNullForm.getVals()[0]), & (vorticity.getVals()[0]));
	addDiffusionSolver->solve(&(tempNullForm.getVals()[0]), & (vorticity.getVals()[0]));
#ifdef printMat
	L.saveVector(vorticity.getVals(),"vort_after","C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vort_after_diffusion.m");
#endif

	star0.mult(tempNullForm.getVals(),vorticity.getVals());

}




//////////////////////////////////////////////////////////////////////////
// dbg & visualisation
//////////////////////////////////////////////////////////////////////////

void fluidSimulation::showDualPositions()
{
	Model::getModel()->setPointCloud(&dualVertices);
}


void fluidSimulation::showFlux2Vel()
{

	//fluidTools::flux2Velocity(flux,velocities, *myMesh);
	updateVelocities();
	//display hack
	Model::getModel()->setVectors(&dualVertices,&velocities);
}





void fluidSimulation::pathTraceAndShow(float howmuch)
{
	//fluidTools::flux2Velocity(flux,velocities, *myMesh);
	updateVelocities();
	pathTraceDualVertices(howmuch);
	Model::getModel()->setPointCloud(&backtracedDualVertices);

	updateBacktracedVelocities();
	Model::getModel()->setVectors(&backtracedDualVertices,&backtracedVelocity);
}

void fluidSimulation::setStreamlines( bool on )
{
	this->showStreamLines = on;
}

void fluidSimulation::setStreamlineLength( int length )
{
	this->streamlineLength = length;
}

void fluidSimulation::setInterpolation( bool on )
{
	this->doInterpolation = on;
}


void fluidSimulation::glDisplayField()
{

	actualizeFPS();
	if(showStreamLines){
		int nrPoints_2 = streamlineLength;//10
		int nrPoints = 2*nrPoints_2;
		
		tuple3f newStart;
		tuple3f temp;
		tuple3f intern_tuple;
		std::vector<float> intern_buff;
		int tempTriangle;
		float t;
		float col;
		int sz = myMesh->getBasicMesh().getFaces().size();
		bool hitBorder;

		glEnable(GL_TEXTURE_1D);
		for(int i = 0; i < line_stripe_starts.size(); i++){

			temp = line_stripe_starts[i];
			tempTriangle = line_strip_triangle[i];
			//speed up: do not animate regions where nothing happens.
			getVelocityFlattened(temp,tempTriangle, intern_tuple,intern_buff);
			if(intern_tuple.norm() < 0.01){
				continue;
			}

			glBegin(GL_LINE_STRIP);
			glTexCoord1f(texPos(age[i] +nrPoints_2,nrPoints));//(0.f+(age[i] + nrPoints_2)%nrPoints)/(nrPoints+1));
			glVertex3fv( (GLfloat *) &temp);

			hitBorder = false;
			for(int j = nrPoints_2+1; j < nrPoints; j++){
				t=0.1;
				while(t>0.0001 && !hitBorder/*tempTriangle >=0*/){
					walkPath(&temp,&tempTriangle,&t,&hitBorder,intern_buff,1);
					glTexCoord1f(texPos(age[i]+ j, nrPoints));//(0.f + (j+age[i])%nrPoints) /(nrPoints+1));
					glVertex3fv( (GLfloat *) &temp);
				}

			}
			glEnd();


			temp = line_stripe_starts[i];
			tempTriangle = line_strip_triangle[i];

			glBegin(GL_LINE_STRIP);
			glTexCoord1f(texPos(age[i] +nrPoints_2,nrPoints));//((0.f+(age[i] + nrPoints_2)%nrPoints)/(nrPoints+1));
			glVertex3fv( (GLfloat *) &temp);
			hitBorder = false;
			for(int j = nrPoints_2 -1; j > 0; j--){
				t=0.1;
				while(t>0.0001 && !hitBorder){
					walkPath(&temp,&tempTriangle,&t,&hitBorder,intern_buff,-1);
					glTexCoord1f(texPos(age[i]+ j, nrPoints));//((0.f+(age[i] + j)%nrPoints)/(nrPoints+1));
					glVertex3fv( (GLfloat *) &temp);
				}
			}
			glEnd();

		/*	age[i]--;
			if(age[i]<0){
				age[i] = maxAge;
			}*/
		}

		//glDisable(GL_LINE_STIPPLE);
		glDisable(GL_TEXTURE_1D);
	}
//#endif //DISPLAY_ISOLINES
}

//////////////////////////////////////////////////////////////////////////
// Helper method that calculates 1d texture positions
//////////////////////////////////////////////////////////////////////////
float fluidSimulation::texPos( int j, int nrPoints )
{
	if(nrPoints < 20){
		nrPoints = nrPoints * (20/nrPoints +1);
	}
	float temp = (j%nrPoints > nrPoints/2? 
		(nrPoints-j%nrPoints)*2.f/nrPoints : 
		(j%nrPoints)*2.f/nrPoints);
	temp = (temp <0 ? 0 :(temp>1?1: temp));
	return temp;
}

//////////////////////////////////////////////////////////////////////////
// Method from colorMap
//////////////////////////////////////////////////////////////////////////
tuple3f fluidSimulation::color( int vertexNr )
{
	assert(vertexNr < vorticity.size());
	float sth = (vorticity.get(vertexNr,1)), sth2, sth3;
	float scale = star0.geta()[0];

	if(showVortNotSpeed){
		sth = abs(vorticity.get(vertexNr,1))*scale/star0.geta()[vertexNr];//(vorticity.get(vertexNr,1)-minVort)/(maxVort-minVort);
		sth = log(100*sth + 1) /(0.7*log(11.0)); // sth = 0 for sth = 0 else > 0, >0.7 for sth > 0.1
		sth2 = (sth > 0.7? sth-0.7:0);
		sth2 = log(10*sth2 + 1)/2; //>1 if sth2 >=1 i.e. sth>1.7 i.e sth > 1
		sth3 = (sth2>0.7?sth2-0.7:0);
		sth3 = log(sth3+1)/3;
		if(vorticity.get(vertexNr,1) < 0){
			sth = 0.3 + sth;///(2*minVort) ;
			return tuple3f(0.3,0.3 + sth2,sth-sth3);
		}
		sth = 0.3+ sth;///(2*maxVort);
		return tuple3f(sth-sth3,0.3+sth2,0.3);
	}
	else{
		vector<int> & nbrs = myMesh->getBasicMesh().getNeighborFaces()[vertexNr];
		sth = 0;
		for(int i = 0; i < nbrs.size(); i++){
			sth += velocities[nbrs[i]].norm();
		}
		sth/= nbrs.size();
		sth = log(sth+1)/2;
		//sth = sth/10; // sth = 0 for sth = 0 else > 0, >0.7 for sth > 0.1
		sth2 = (sth > 1? sth-1:0);
		sth = sth - sth2;
		//sth2 /= 10; //>1 if sth2 >=1 i.e. sth>1.7 i.e sth > 1
		sth3 = (sth2>1?sth2-1:0);
		sth2 = sth2 - sth3;
		//sth3/=10;
		return tuple3f(sth2,sth-sth2, 1-sth);
	}

}

std::string fluidSimulation::additionalInfo( void )
{
	throw std::runtime_error("The method or operation is not implemented.");
}


tuple3f fluidSimulation::randPoint( int triangle )
{
	tuple3i & tr = myMesh->getBasicMesh().getFaces()[triangle];
	std::vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	float a = (float)rand()/numeric_limits<float>::max();
	float b = (float)rand()/numeric_limits<float>::max();
	float c = (float)rand()/numeric_limits<float>::max();
	float sum = a+b+c;
	a/=sum;
	b/=sum;
	c/=sum;

	return (verts[tr.a] * a) + (verts[tr.b]*b) + (verts[tr.c]*c);

}

float fluidSimulation::getSimTime()
{
	return simulationtime;
}
float fluidSimulation::getFPS()
{
	return fps;
}

void fluidSimulation::actualizeFPS()
{
	fps = 1000.f/lastFrame.elapsed();
	lastFrame.restart();
}

void fluidSimulation::testFlux()
{
	std::vector<tuple3i> & fcs = myMesh->getBasicMesh().getFaces();
	std::vector<tuple2i> & edges = * myMesh->getHalfedges();
	std::vector<tuple3i> & f2e = * myMesh->getFace2Halfedges();

	float tot;
	for(int  i= 0; i < fcs.size(); i++){
		tot = 0;
		tot += flux.get(f2e[i].a, fcs[i].orientation(edges[f2e[i].a]));
		tot += flux.get(f2e[i].b, fcs[i].orientation(edges[f2e[i].b]));
		tot += flux.get(f2e[i].c, fcs[i].orientation(edges[f2e[i].c]));

		if(tot >0.001 || tot <-0.001){
			assert(false);
		}
	}
}

void fluidSimulation::showHarmonicField()
{
		Model::getModel()->setVectors(&dualVertices,&harmonicVelocities);
}	

vector<tuple3f> & fluidSimulation::getVelocities()
{
	return this->velocities;
}

vector<tuple3f> & fluidSimulation::getBacktracedVelocities()
{
	return this->backtracedVelocity;
}

nullForm & fluidSimulation::getVorticity()
{
	return vorticity;
}

bool fluidSimulation::checkAllDualVerticesInside()
{
	tuple3f a,b,c,n;
	vector<tuple3f> & fcNormals = myMesh->getBasicMesh().getFaceNormals();
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	vector<tuple3i> & fcs = myMesh->getBasicMesh().getFaces();
	float temp, eps = 0.01;
	
	bool allOk = true;
	for(int i = 0; i < dualVertices.size(); i++){
		a.set(verts[fcs[i].a]);
		b.set(verts[fcs[i].b]);
		c.set(verts[fcs[i].c]);
		n.set(fcNormals[i]);
		
		if( (temp = n.cross(b-a).dot(dualVertices[i] -a)) < 0){
			if(temp > -eps){
				dualVertices[i] = (a+b)*0.5f;
				continue;
			}
			allOk = false;
//			break;
		}
		if((temp = n.cross(c-b).dot(dualVertices[i] -b)) < 0){
			if(temp > -eps){
				dualVertices[i] = (c+b)*0.5f;
				continue;
			}
			allOk = false;
//			break;
		}
		if((temp = n.cross(a-c).dot(dualVertices[i] -c)) < 0){
			if(temp > -eps){
				dualVertices[i] = (a+c)*0.5f;
				continue;
			}
			allOk = false;
//			break;
		}

	}

	return allOk;
}

std::vector<tuple3f> & fluidSimulation::getDualVertices()
{
	return this->dualVertices;
}


//adapts the two matrices such that inner borders are treated as borders to one
// additional face and the vorticity around this face should be 0.
void fluidSimulation::adaptMatrices_zeroTotalBorderVort( vector<vector<int>> &brdr, pardisoMatrix &star0inv,pardisoMatrix &duald1 )
{
	int sz;

	pardisoMatrix borderVerticesT;
	vector<double> vals;
	for(int i = 1; i < brdr.size(); i++){
		vector<int> indices = brdr[i];
		sort(indices.begin(), indices.end());

		vals.reserve(brdr[i].size());
		for(int j = 0; j < brdr[i].size(); j++){
			vals.push_back(1);
		}
		borderVerticesT.addLine(indices, vals);
		borderVerticesT.forceNrColumns(myMesh->getBasicMesh().getVertices().size());

		(borderVerticesT * duald1).getLine(0,indices,vals);
		duald1.addLine(indices,vals);
	}

	//adapt matrix....
	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
		for(int j = 0; j < sz;j++){
			//'unconstrained'
			star0inv.setLineToZero(brdr[i][j]);
		}
	}
	for(int i = 1; i < brdr.size(); i++){
		vector<int> bla;
		vector<double> bla2;
		bla.push_back(star0inv.getn());
		bla2.push_back(1000000);
		star0inv.addLine(bla,bla2);
	}
}

void fluidSimulation::showVorticity( bool param1 )
{
	this->showVortNotSpeed = param1;
}









