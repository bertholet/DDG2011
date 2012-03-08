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

//#define printMat
//#define printMat_harmonicFlux

fluidSimulation::fluidSimulation( meshMetaInfo * mesh ):
flux(*mesh), vorticity(*mesh), L_m1Vorticity(*mesh), tempNullForm(*mesh), forceFlux(*mesh)
{
	myMesh = mesh;
	simulationtime = 0;

	dualMeshTools::getDualVertices(*mesh, dualVertices);
	assert(checkAllDualVerticesInside());

	backtracedDualVertices = dualVertices; //this copies everything.
	backtracedVelocity = dualVertices; // just to have the right dimension



	//for visualisation
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


	//for Backtracing
	triangle_btVel.reserve(dualVertices.size());
	velocities.reserve(dualVertices.size());
	harmonicVelocities.reserve(dualVertices.size());
	for(int i = 0; i < dualVertices.size(); i++){
		triangle_btVel.push_back(-1);
		velocities.push_back(tuple3f());
		harmonicVelocities.push_back(tuple3f());
	}


	//matrix set up.
	d0 =DDGMatrices::d0(*myMesh);
	//borderdiff is zero if there is no border.
//	dt_star1 = (DDGMatrices::dual_d1(*myMesh) +DDGMatrices::dual_d1_borderdiff(*myMesh))* DDGMatrices::star1(*myMesh);
	dt_star1 = ((DDGMatrices::dual_d1(*myMesh))* DDGMatrices::star1(*myMesh)) + DDGMatrices::dual_d1star1_borderdiff(*myMesh);

	L = dt_star1*d0;/*DDGMatrices::dual_d1(*myMesh) * DDGMatrices::star1(*myMesh) * d0;*/

	//add stuff for 0 on border condition. Might be a bad idea.
	pardisoMatrix border = DDGMatrices::onesBorder(myMesh->getBorder(),L.getn(), L.getn());
	border *= 0.0001;
	L = L + border;

	star0 = DDGMatrices::star0(*myMesh);

	this->setStepSize(0.05);
	this->setViscosity(0);


#ifdef printMat
	L.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/laplace0.m");
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
	star0_min_vhl = L; 
	star0_min_vhl *= viscosity*timeStep;

#ifdef printMat
	star0_min_vhl.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vtL0.m");
#endif

	//the final matrix
	star0_min_vhl = star0 - star0_min_vhl; //was star0 not id0. WAS MINUS !!! Now IS minus because of star1 assumption

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



oneForm fluidSimulation::setHarmonicFlow( vector<tuple3f> & borderConstraints )
{
	if(myMesh->getBorder().size() == 0){
		//only for bordered meshs.
		return oneForm(*myMesh);
	}
	//compute a border adapted least square formulation laplacian.
	pardisoMatrix d1 = DDGMatrices::d1(*myMesh);
	pardisoMatrix star0inv = DDGMatrices::star0(*myMesh);
	star0inv.elementWiseInv(0);
	pardisoMatrix Lflux = (pardisoMatrix::transpose(d1)*DDGMatrices::star2(*myMesh)*d1) //* 100
		+ pardisoMatrix::transpose(dt_star1)*star0inv*dt_star1;




	// setting flux constraints
	float weight = 10000;
	
	vector<vector<int>> & brdr = myMesh->getBorder();
	vector<tuple2i> & edgs = * myMesh->getHalfedges();
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	oneForm constFlux(*myMesh), harmonicFlux(*myMesh);
	vector<double> buff;
	vector<double> fluxConstr = harmonicFlux.getVals(); //init to zero, right size.
	int sz,edgeId;
	tuple2i edge;

#ifdef printMat_harmonicFlux
	std:vector<int> borderEdges;
	borderEdges.reserve(edgs.size());
#endif

	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
		constFlux.initToConstFlux(borderConstraints[i]);
		Lflux.mult(constFlux.getVals(), buff,true);
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

	/*constFlux.initToConstFlux(tuple3f());
	//debug stuff..
	Lflux.mult(constFlux.getVals(), buff,true);
	for(int i = 10; i < 50; i++){
		Lflux.add(i,i,weight);
		fluxConstr[i] = tuple3f(1.f,1.f,1.f).dot(verts[edgs[i].b] -verts[edgs[i].a]) * weight;// + buff[edgeId];
	}

	for(int i = 209; i < 259; i++){
		Lflux.add(i,i,weight);
		fluxConstr[i] = tuple3f(1.f,1.f,1.f).dot(verts[edgs[i].b] -verts[edgs[i].a]) * weight;// + buff[edgeId];
	}

	for(int i = 2009; i < 2059; i++){
		Lflux.add(i,i,weight);
		fluxConstr[i] = tuple3f(1.f,1.f,1.f).dot(verts[edgs[i].b] -verts[edgs[i].a]) * weight;// + buff[edgeId];
	}
	for(int i = 6009; i < 6059; i++){
		Lflux.add(i,i,weight);
		fluxConstr[i] = tuple3f(1.f,1.f,1.f).dot(verts[edgs[i].b] -verts[edgs[i].a]) * weight;// + buff[edgeId];
	}
	for(int i = 14009; i < 14059; i++){
		Lflux.add(i,i,weight);
		fluxConstr[i] = tuple3f(1.f,1.f,1.f).dot(verts[edgs[i].b] -verts[edgs[i].a]) * weight;// + buff[edgeId];
	}*/

	//solve for the harmonic flux.
	pardisoSolver solver(pardisoSolver::MT_ANY, pardisoSolver::SOLVER_DIRECT,15);
	solver.setMatrix(Lflux,1);
	solver.setStoreResultInB(false);
	solver.solve(& (harmonicFlux.getVals()[0]), & (fluxConstr[0]));
	//already stored in harmonicFlux.

	fluidTools::flux2Velocity(harmonicFlux,harmonicVelocities, *myMesh);

	//fluidTools::dirs2Flux(harmonicVelocities, harmonicFlux,*myMesh);

	updateVelocities();

#ifdef printMat_harmonicFlux
	for(int i = 0; i < fluxConstr.size(); i++){
		fluxConstr[i] /= weight;
	}
	Lflux.saveVector(fluxConstr, "flux_constraint","C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/fluxConstr.m");
	Lflux.saveVector(borderEdges, "border_edges","C:/Users/bertholet/Dropbox/To Delete/harmonicFlowTest/borderEdges.m");
#endif // printMat_harmonicFlux

	return harmonicFlux;

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
	pathTraceDualVertices(timeStep); 

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

//testFlux();

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
	assert(dir.x == dir.x && dir.y == dir.y && dir.z == dir.z);
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	tuple3i& tr = myMesh->getBasicMesh().getFaces()[triangle];
	tuple3f & a = verts[tr.a];
	tuple3f & b = verts[tr.b];
	tuple3f & c = verts[tr.c];


	tuple3f n = (b-a).cross(c-a);
	n.normalize();
	//normals pointing into the triangle
	tuple3f n1 = (a-b).cross(n);
	tuple3f n2 = (b-c).cross(n);
	tuple3f n3 = (c-a).cross(n);

	//normals on the sides.
/*	tuple3f n1 = dualVertices[triangle]-(a+b)*0.5f;
	tuple3f n2 = dualVertices[triangle]-(b+c)*0.5f;
	tuple3f n3 = dualVertices[triangle]-(c+a)*0.5f;*/
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

	solver.solve(&(L_m1Vorticity.getVals()[0]), & (vorticity.getVals()[0]));
	d0.mult(L_m1Vorticity.getVals(),flux.getVals());
	assert(flux.getVals()[flux.getVals().size()/2] < 10E10 &&
		flux.getVals()[flux.getVals().size()/2] > -10E10 );
}


void fluidSimulation::flux2Vorticity()
{
	fluidTools::flux2Vorticity(flux,vorticity, *myMesh, dt_star1);
}





tuple3f fluidSimulation::getVelocityFlattened( tuple3f & pos, int actualTriangle, bool useHarmonicField)
{

	if(actualTriangle <0){
		return tuple3f();
	}
	assert(actualTriangle >=0);
	std::vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	std::vector<float> weights;
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
		// NAN....
		assert(false);
	}

	//determine weights;
	fluidTools::bariCoords(pos,dualFace,dualVertices, weights, *myMesh);
	// dualVertices of dualFace;
	std::vector<int> & dualVertIDs = myMesh->getBasicMesh().getNeighborFaces()[dualFace];

	tuple3f curvNormal = (*myMesh->getCurvNormals())[dualFace];
	if(myMesh->getBorder().size() != 0){
		//on bordered meshs the stuff with the curv normal does not work.
		//for now assum that meshes are flat if they have a border
		for(int i = 0; i < weights.size(); i++){
			assert(weights[i]== weights[i]);
			result += (velocities[dualVertIDs[i]]/*+harmonicVelocities[dualVertIDs[i]]*/)*weights[i];
			if(!useHarmonicField){
				result -= harmonicVelocities[dualVertIDs[i]]*weights[i];
			}
		}
	}
	else{
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
	return result;
}


//////////////////////////////////////////////////////////////////////////
//Bugs:
//maybe overly imprecise on curved surfaces because of taking euclidean distance
// which is a CRUDE approx of the geodesic distance (which is implicitely
// used in the dual edge edge ratios)
//////////////////////////////////////////////////////////////////////////
void fluidSimulation::backtracedVorticity()
{
	std::vector<std::vector<int>> & dualf2v = myMesh->getBasicMesh().getNeighborFaces();
	std::vector<double> & vort = vorticity.getVals();
	bool anyVertexOutside;

	updateBacktracedVelocities();
	double temp;
	int sz;
	for(int i = 0; i < vorticity.size();i++){
		temp = 0;
		anyVertexOutside= false;

		//the dual vertices surrounding the face.
		std::vector<int> & dualV = dualf2v[i];
		sz = dualV.size();
		for(int j = 0; j < sz; j++){

			if(triangle_btVel[dualV[j]] == -1){
				anyVertexOutside = true;
				continue; // velocity is assumed to be zero on this edge
			}
			// was += now -= because the dual edges are oriented in the oposite way of following the border of the
			// one ring of vertex.!!!
			temp -= 0.5* ((backtracedVelocity[dualV[j]] + backtracedVelocity[dualV[(j+1)%sz]]).dot(
				backtracedDualVertices[dualV[(j+1)%sz]] - backtracedDualVertices[dualV[j]])); 
		}
		vort[i] =temp;

//		if(anyVertexOutside){
//			vort[i] = 0;
//		}
	}
}

void fluidSimulation::updateBacktracedVelocities()
{
	for(int i = 0; i < backtracedVelocity.size(); i++){
		backtracedVelocity[i] = getVelocityFlattened(backtracedDualVertices[i],triangle_btVel[i], true);
	}
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
	pardisoSolver solver(pardisoSolver::MT_ANY,pardisoSolver::SOLVER_DIRECT,3);
	solver.setMatrix(star0_min_vhl,1);
//	solver.setStoreResultInB(true);

#ifdef printMat
	L.saveVector(vorticity.getVals(),"vort_before","C:/Users/bertholet/Dropbox/To Delete/fluidsim dbg/vort_before_diffusion.m");
#endif
	solver.solve(&(tempNullForm.getVals()[0]), & (vorticity.getVals()[0]));
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


	/*tuple3f temp;
	int tempTriangle;
	for(int i = 0; i < line_stripe_starts.size(); i++){

		temp = line_stripe_starts[i];
		tempTriangle = line_strip_triangle[i];
		//speed up: do not animate regions where nothing happens.

		glBegin(GL_LINE);
		glVertex3fv( (GLfloat *) &temp);
		temp = getVelocityFlattened(temp,tempTriangle);
		glVertex3fv( (GLfloat *) &temp);
		glEnd();

	}*/
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




