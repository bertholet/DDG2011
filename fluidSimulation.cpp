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

fluidSimulation::fluidSimulation( meshMetaInfo * mesh ):
flux(*mesh), vorticity(*mesh), L_m1Vorticity(*mesh), tempNullForm(*mesh), forceFlux(*mesh)
{
	myMesh = mesh;
	dualMeshTools::getDualVertices(*mesh, dualVertices);
	backtracedDualVertices = dualVertices; //hope this copies everything.
	backtracedVelocity = dualVertices; // just to have the right dimension
	line_stripe_starts = dualVertices; // for visualisation

	triangle_btVel.reserve(dualVertices.size());
	line_strip_triangle.reserve(dualVertices.size());
	velocities.reserve(dualVertices.size());
	for(int i = 0; i < dualVertices.size(); i++){
		triangle_btVel.push_back(-1);
		line_strip_triangle.push_back(i);
		velocities.push_back(tuple3f());
	}


	//the one in the comments would be if vorticity was defined on the edges in 3d.
	//L = DDGMatrices::d0(*myMesh) * DDGMatrices::delta1(*myMesh) + DDGMatrices::delta2(*myMesh) * DDGMatrices::d1(*myMesh);
	d0 =DDGMatrices::d0(*myMesh);
	L = DDGMatrices::delta1(*myMesh) * d0;
	dt_star1 = (DDGMatrices::id0(*myMesh) % DDGMatrices::d0(*myMesh)) * DDGMatrices::star1(*myMesh);
	star0_ = DDGMatrices::star0(*myMesh);
	star0_inv = star0_;
	star0_inv.elementWiseInv(0.00001f);

	this->setStepSize(0.05);
	this->setViscosity(0);
}

fluidSimulation::~fluidSimulation(void)
{
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
	star0_min_vhl = L;
	star0_min_vhl *= viscosity*timeStep;

	//the final matrix
	star0_min_vhl = DDGMatrices::id0(*myMesh) - star0_min_vhl; //was star0 not id0
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
	fluidTools::dirs2Flux(dirs,flux,*myMesh, dualVertices);
}

//////////////////////////////////////////////////////////////////////////
// converts the directions tu flux and uses this flux as force (e.g. stirring)
// the same restriction as for setFlux hold.
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::setForce(vector<tuple3f> & dirs)
{
	fluidTools::dirs2Flux(dirs,forceFlux,*myMesh, dualVertices);
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
void fluidSimulation::oneStep( float howmuuch )
{
	pathTraceDualVertices(howmuuch); //note: the second u is critical for success.

	Model::getModel()->setPointCloud(&backtracedDualVertices);

	updateBacktracedVelocities();

	//Model::getModel()->setVectors(&backtracedDualVertices,&backtracedVelocity);

	//dbg only
	//	flux2Vorticity();
	//	std::vector<double> old_vorts_for_debug = vorticity.getVals();
	//gbd

	backtracedVorticity();

	addForces2Vorticity(howmuuch);

	addDiffusion2Vorticity();

	vorticity2Flux();
	updateVelocities();

	Model::getModel()->setVectors(&dualVertices,&velocities);
}

void fluidSimulation::walkPath( tuple3f * pos, int * triangle, float * t, int direction )
{
	tuple3f dir = getVelocityFlattened(*pos,*triangle);
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
		*triangle = temp;
	}
}


float fluidSimulation::maxt( tuple3f & pos, int triangle, tuple3f & dir, tuple3f & cutpos, tuple2i & edge )
{
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	tuple3i& tr = myMesh->getBasicMesh().getFaces()[triangle];
	tuple3f & a = verts[tr.a];
	tuple3f & b = verts[tr.b];
	tuple3f & c = verts[tr.c];
	//normals on the sides.
	tuple3f n1 = dualVertices[triangle]-(a+b)*0.5f;
	tuple3f n2 = dualVertices[triangle]-(b+c)*0.5f;
	tuple3f n3 = dualVertices[triangle]-(c+a)*0.5f;
	n1.normalize();
	n2.normalize();
	n3.normalize();
	float t= n1.dot(a-pos)/n1.dot(dir);
	float t2 = n2.dot(b-pos)/n2.dot(dir);
	float t3 = n3.dot(c-pos)/n3.dot(dir);
	float max_t = numeric_limits<float>::infinity();


	if(t==t && t>=-0.00001 && dir.dot(n1)<0){
		max_t = t;
	}
	if(t2==t2 && t2>= -0.00001 && t2<max_t && dir.dot(n2)<0){
		max_t = t2;
	}
	if(t3==t3 && t3>= -0.00001 && t3<max_t && dir.dot(n3)<0){
		max_t = t3;
	}
	
//	assert(max_t == max_t && max_t!=numeric_limits<float>::infinity() &&max_t>=0);

	cutpos.set(pos+dir*max_t);
	if(t==max_t){
		edge.set(tr.a,tr.b);
	}
	else if(t2==max_t){
		edge.set(tr.b, tr.c);
	}
	else if(t3==max_t){
		edge.set(tr.c, tr.a);
	}

	return max_t;
}


void fluidSimulation::pathTraceDualVertices( float t )
{
	std::vector<tuple3i> & fcs = myMesh->getBasicMesh().getFaces();
	backtracedDualVertices = dualVertices;
	int triangle;
	int nrIterations = t/0.05;
	float changed_t;
	for(int i = 0; i < backtracedDualVertices.size(); i++){
		triangle = i;
		for(int j = 1; j < nrIterations;j++){
			changed_t = t/nrIterations;
			//triangle = -1 => outside of mesh
			while(changed_t > 0.0000001 && triangle!=-1){
				walkPath(&(backtracedDualVertices[i]), &triangle,&changed_t);
			}
		}
		triangle_btVel[i]=triangle;
	}
}



void fluidSimulation::vorticity2Flux()
{
	pardisoSolver solver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,3);
	solver.setMatrix(L,1);
	star0_inv.mult((vorticity.getVals()),(tempNullForm.getVals()));

	solver.solve(&(L_m1Vorticity.getVals()[0]), & (tempNullForm.getVals()[0]));
	d0.mult(L_m1Vorticity.getVals(),flux.getVals());
}


void fluidSimulation::flux2Vorticity()
{
	fluidTools::flux2Vorticity(flux,vorticity, *myMesh, dt_star1);
}




/*
tuple3f fluidSimulation::project( tuple3f& velocity,int actualFc)
{
	tuple3i & actualFace =myMesh->getBasicMesh().getFaces()[actualFc];
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	tuple3f & a =verts[actualFace.a];
	tuple3f & b =verts[actualFace.b];
	tuple3f & c =verts[actualFace.c];
	tuple3f b_a = (b-a);
	b_a.normalize();
	tuple3f c_a = (c-a);
	c_a -= b_a * (c_a.dot(b_a));
	c_a.normalize();

	//assert(c_a.dot(b_a) < 0.000001 &&c_a.dot(b_a) >- 0.000001);
	tuple3f result = c_a * (velocity.dot(c_a)) + b_a * (velocity.dot(b_a));
	//result*=velocity.norm()/result.norm();

	float tmp = ((b-a).cross(c-a)).dot(result);
	assert(tmp < 0.00001&& tmp >-0.00001);

	return result;

}*/


tuple3f fluidSimulation::getVelocityFlattened( tuple3f & pos, int actualTriangle)
{

	assert(actualTriangle >=0);
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	vector<float> weights;
	tuple3f result;
	tuple3i & tr = myMesh->getBasicMesh().getFaces()[actualTriangle];
	//determine actual dual Face
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
		assert(false);
	}

	//determine weights;
	fluidTools::bariCoords(pos,dualFace,dualVertices, weights, *myMesh);
	// dualVertices of dualFace;
	std::vector<int> & dualVertIDs = myMesh->getBasicMesh().getNeighborFaces()[dualFace];

	tuple3f curvNormal = (*myMesh->getCurvNormals())[dualFace];
	curvNormal.normalize();
	tuple3f triangleNormal = (verts[tr.b]-verts[tr.a]).cross(verts[tr.c]-verts[tr.a]);

	for(int i = 0; i < weights.size(); i++){
		//result += turnVelocityInRightPlane(velocities[dualVertIDs[i]], dualVertIDs[i], actualTriangle)*weights[i];
		result += (velocities[dualVertIDs[i]] - curvNormal * (curvNormal.dot(velocities[dualVertIDs[i]])) )*weights[i];
	}

	result = result - curvNormal * (result.dot(triangleNormal) / curvNormal.dot(triangleNormal));

	return result;
}


//////////////////////////////////////////////////////////////////////////
//Bugs:
//maybe overly imprecise, maybe because of orientation errors or sth?
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::backtracedVorticity()
{
	std::vector<std::vector<int>> & dualf2v = myMesh->getBasicMesh().getNeighborFaces();
	std::vector<double> & vort = vorticity.getVals();

	updateBacktracedVelocities();
	double temp;
	int sz;
	for(int i = 0; i < vorticity.size();i++){
		temp = 0;
		//the dual vertices surrounding the face.
		std::vector<int> & dualV = dualf2v[i];
		sz = dualV.size();
		for(int j = 0; j < sz; j++){
			temp += 0.5* ((backtracedVelocity[dualV[j]] + backtracedVelocity[dualV[(j+1)%sz]]).dot(
				backtracedDualVertices[dualV[(j+1)%sz]] - backtracedDualVertices[dualV[j]]));
		}
		vort[i] =temp;
	}
}

void fluidSimulation::updateBacktracedVelocities()
{
	for(int i = 0; i < backtracedVelocity.size(); i++){
		backtracedVelocity[i] = getVelocityFlattened(backtracedDualVertices[i],triangle_btVel[i]);
	}
}






void fluidSimulation::updateVelocities()
{
	fluidTools::flux2Velocity(flux,velocities, *myMesh);
}


void fluidSimulation::addForces2Vorticity(float timeStep)
{
	dt_star1.mult(forceFlux.getVals(),tempNullForm.getVals());
	vorticity.add(tempNullForm, timeStep);
}


void fluidSimulation::addDiffusion2Vorticity()
{
	pardisoSolver solver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,3);
	solver.setMatrix(star0_min_vhl,1);
	
	solver.solve(&(tempNullForm.getVals()[0]), & (vorticity.getVals()[0]));

//	star0_.mult(tempNullForm.getVals(),vorticity.getVals());
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

	fluidTools::flux2Velocity(flux,velocities, *myMesh);
	//display hack
	Model::getModel()->setVectors(&dualVertices,&velocities);
}





void fluidSimulation::pathTraceAndShow(float howmuch)
{
	fluidTools::flux2Velocity(flux,velocities, *myMesh);
	pathTraceDualVertices(howmuch);
	Model::getModel()->setPointCloud(&backtracedDualVertices);

	updateBacktracedVelocities();
	Model::getModel()->setVectors(&backtracedDualVertices,&backtracedVelocity);
}

void fluidSimulation::glDisplayField()
{

	static GLushort forward_pattern = 0x3F3F;
	static GLushort backward_pattern = 0xFCFC;

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(8, forward_pattern);
	glColor3f(0.f,0.f,0.f);


	//forward_pattern = (forward_pattern << 1) | (forward_pattern >> 15);
	//backward_pattern = (backward_pattern << 15) | (backward_pattern >> 1);


	tuple3f newStart;
	tuple3f temp;
	int tempTriangle;
	float t;
	for(int i = 0; i < line_stripe_starts.size(); i++){
		temp = line_stripe_starts[i];
		tempTriangle = line_strip_triangle[i];
		glBegin(GL_LINE_STRIP);
		for(int j = 0; j < 5; j++){
			glVertex3fv( (GLfloat *) &temp);
			t=0.3;
			while(t>0.0001 && tempTriangle >=0){
				walkPath(&temp,&tempTriangle,&t,1);
			}

			if(j==0){
				line_stripe_starts[i]= temp;
				line_strip_triangle[i] = tempTriangle;
			}
		}
		glEnd();
	}

	glDisable(GL_LINE_STIPPLE);
}
