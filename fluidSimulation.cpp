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

#define printMat

fluidSimulation::fluidSimulation( meshMetaInfo * mesh ):
flux(*mesh), vorticity(*mesh), L_m1Vorticity(*mesh), tempNullForm(*mesh), forceFlux(*mesh)
{
	myMesh = mesh;
	simulationtime = 0;

	dualMeshTools::getDualVertices(*mesh, dualVertices);
	backtracedDualVertices = dualVertices; //hope this copies everything.
	backtracedVelocity = dualVertices; // just to have the right dimension


	//////////////////////////////////////////////////////////////////////////
	//for visualisation
	//////////////////////////////////////////////////////////////////////////
	line_strip_triangle.reserve(dualVertices.size());
	line_stripe_starts.reserve(dualVertices.size()); // for visualisation
	age.reserve(dualVertices.size());
	maxAge = 200;
	int noFaces = myMesh->getBasicMesh().getFaces().size();
	srand(0);
	for(int i = 0; i < noFaces; i++){

		line_strip_triangle.push_back(i);//(rand()%noFaces);
		line_stripe_starts.push_back(/**/randPoint(i));//*/dualVertices[line_strip_triangle[i]]);
		age.push_back(rand()%maxAge);
	}


//////////////////////////////////////////////////////////////////////////
	//for Backtracing
	//////////////////////////////////////////////////////////////////////////
	triangle_btVel.reserve(dualVertices.size());
	velocities.reserve(dualVertices.size());
	for(int i = 0; i < dualVertices.size(); i++){
		triangle_btVel.push_back(-1);
		velocities.push_back(tuple3f());
	}


	//the one in the comments would be if vorticity was defined on the edges in 3d.
	//L = DDGMatrices::d0(*myMesh) * DDGMatrices::delta1(*myMesh) + DDGMatrices::delta2(*myMesh) * DDGMatrices::d1(*myMesh);
	d0 =DDGMatrices::d0(*myMesh);
	L = DDGMatrices::delta1(*myMesh) * d0;
	dt_star1 = (DDGMatrices::id0(*myMesh) % DDGMatrices::d0(*myMesh)) * DDGMatrices::star1(*myMesh);

	star0_inv = DDGMatrices::star0(*myMesh);
	star0_inv.elementWiseInv(0.00001f);

	L_dual = DDGMatrices::star0(*myMesh) * L * star0_inv;//dt_star1 * DDGMatrices::d0(*myMesh) * star0_inv;

	this->setStepSize(0.05);
	this->setViscosity(0);

	//////////////////////////////////////////////////////////////////////////
	//dbg
	//////////////////////////////////////////////////////////////////////////
#ifdef printMat
	L.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/laplace0.m");
	L_dual.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/laplace_dual0.m");
#endif

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
	star0_min_vhl = L_dual; // was L once
	star0_min_vhl *= viscosity*timeStep;

#ifdef printMat
	star0_min_vhl.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vtL0.m");
#endif

	//the final matrix
	star0_min_vhl = DDGMatrices::id0(*myMesh) - star0_min_vhl; //was star0 not id0. 

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
void fluidSimulation::oneStep()
{
	pathTraceDualVertices(timeStep); //note: the second u is critical for success.

	//Model::getModel()->setPointCloud(&backtracedDualVertices);

	updateBacktracedVelocities();

	//Model::getModel()->setVectors(&backtracedDualVertices,&backtracedVelocity);

	//dbg only
	//	flux2Vorticity();
	//	std::vector<double> old_vorts_for_debug = vorticity.getVals();
	//gbd

	backtracedVorticity();

	addForces2Vorticity(timeStep);

	addDiffusion2Vorticity();

	vorticity2Flux();
	updateVelocities();
	simulationtime += timeStep;

	Model::getModel()->setVectors(&dualVertices,&velocities, false);
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
	solver.setStoreResultInB(true);

#ifdef printMat
	L.saveVector(vorticity.getVals(),"vort_before","C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vort_before_diffusion.m");
#endif
	solver.solve(&(tempNullForm.getVals()[0]), & (vorticity.getVals()[0]));
#ifdef printMat
	L.saveVector(vorticity.getVals(),"vort_after","C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vort_after_diffusion.m");
#endif

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
	int nrPoints_2 = 10;
	int nrPoints = 2*nrPoints_2;
	
	tuple3f newStart;
	tuple3f temp;
	int tempTriangle;
	float t;
	float col;
	int sz = myMesh->getBasicMesh().getFaces().size();

	actualizeFPS();
	glEnable(GL_TEXTURE_1D);
	for(int i = 0; i < line_stripe_starts.size(); i++){

		temp = line_stripe_starts[i];
		tempTriangle = line_strip_triangle[i];
		//speed up: do not animate regions where nothing happens.
		if(getVelocityFlattened(temp,tempTriangle).norm() < 0.01){
			continue;
		}

		glBegin(GL_LINE_STRIP);
		glTexCoord1f(texPos(age[i] +nrPoints_2,nrPoints));//(0.f+(age[i] + nrPoints_2)%nrPoints)/(nrPoints+1));
		glVertex3fv( (GLfloat *) &temp);

		for(int j = nrPoints_2+1; j < nrPoints; j++){
			t=0.1;
			while(t>0.0001 && tempTriangle >=0){
				walkPath(&temp,&tempTriangle,&t,1);
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
		for(int j = nrPoints_2 -1; j > 0; j--){
			t=0.1;
			while(t>0.0001 && tempTriangle >=0){
				walkPath(&temp,&tempTriangle,&t,-1);
				glTexCoord1f(texPos(age[i]+ j, nrPoints));//((0.f+(age[i] + j)%nrPoints)/(nrPoints+1));
				glVertex3fv( (GLfloat *) &temp);
			}
		}
		glEnd();

		age[i]--;
		if(age[i]<0){
			age[i] = maxAge;
		}
	}

	//glDisable(GL_LINE_STIPPLE);
	glDisable(GL_TEXTURE_1D);
}

//////////////////////////////////////////////////////////////////////////
// Helper method that calculates 1d texture positions
//////////////////////////////////////////////////////////////////////////
float fluidSimulation::texPos( int j, int nrPoints )
{
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
	float sth = vorticity.get(vertexNr,1);
	sth = (sth>0? sth: -sth);
	sth = (sth<0.2?0.2:(sth>0.8?0.8:sth));
	return tuple3f(0.2f,0.2f,sth);
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
