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

fluidSimulation::fluidSimulation( meshMetaInfo * mesh ):
flux(*mesh), vorticity(*mesh)
{
	myMesh = mesh;
	dualMeshTools::getDualVertices(*mesh, dualVertices);
	backtracedDualVertices = dualVertices; //hope this copies everything.
}

fluidSimulation::~fluidSimulation(void)
{
}

void fluidSimulation::walkPath( tuple3f * pos, int * triangle, float * t )
{
	tuple3f dir = getVelocityFlattened(*pos,*triangle,myMesh->getBasicMesh().getFaces()[*triangle]);
	dir*=-1;
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

tuple3f fluidSimulation::getVelocity( tuple3f & pos, int actualTriangle, tuple3i & tr )
{
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	vector<float> weights;
	tuple3f result;
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

	for(int i = 0; i < weights.size(); i++){
		//result += turnVelocityInRightPlane(velocities[dualVertIDs[i]], dualVertIDs[i], actualTriangle)*weights[i];
		result += velocities[dualVertIDs[i]] *weights[i];
	}

	result = project(result, actualTriangle);
	return result;
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

//////////////////////////////////////////////////////////////////////////
//dbg vals
//////////////////////////////////////////////////////////////////////////

/*	float n1_dot_ab = n1.dot(b-a);
	float n2_dot_bc = n2.dot(c-b);
	float n3_dot_ca = n3.dot(a-c);

	float d_dot_n1 = dir.dot(n1);
	float d_dot_n2 = dir.dot(n2);
	float d_dot_n3 = dir.dot(n3);*/
//////////////////////////////////////////////////////////////////////////
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

void fluidSimulation::showDualPositions()
{
	Model::getModel()->setPointCloud(&dualVertices);
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
	}
}

void fluidSimulation::pathTraceAndShow(float howmuch)
{
	fluidTools::flux2Velocity(flux,velocities, *myMesh);
	pathTraceDualVertices(howmuch);
	Model::getModel()->setPointCloud(&backtracedDualVertices);
}

void fluidSimulation::setFlux( oneForm & f )
{
	assert(f.getMesh() == myMesh);
	flux = f;
}

void fluidSimulation::showFlux2Vel()
{

	fluidTools::flux2Velocity(flux,velocities, *myMesh);
	//display hack
	Model::getModel()->setVectors(&dualVertices,&velocities);
}

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

}


tuple3f fluidSimulation::getVelocityFlattened( tuple3f & pos, int actualTriangle, tuple3i & tr )
{
	vector<tuple3f> & verts = myMesh->getBasicMesh().getVertices();
	vector<float> weights;
	tuple3f result;
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
