#include "VectorField.h"
#include "Model.h"
#include "meshMetaInfo.h"
#include <vector>
#include "matrixFactory.h"
#include "matrixf.h"
#include "stdafx.h"
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include "vectorFieldTools.h"

VectorField::VectorField( mesh * aMesh, tuple3f & dir)
{
	meshMetaInfo * info = Model::getModel()->getMeshInfo();
	//info->
//	displayLength = 0.3f;
	edges = info->getHalfedges();
	fc2he = info->getFace2Halfedges();
	faces = &(aMesh->getFaces());
	vertices = &(aMesh->getVertices());
	myMesh = aMesh;


	oneForm.reserve(edges->size());
	for(int i = 0; i < edges->size(); i++){
		oneForm.push_back(0.f);
	}

	for(int i = 0; i < aMesh->faces.size(); i++){
		setOneForm(i,dir);
	}
}

VectorField::VectorField( mesh * aMesh )
{
	meshMetaInfo * info = Model::getModel()->getMeshInfo();
	//info->
//		displayLength = 0.3f;
	edges = info->getHalfedges();
	fc2he = info->getFace2Halfedges();
	faces = &(aMesh->getFaces());
	vertices = &(aMesh->getVertices());
	myMesh = aMesh;

	oneForm.reserve(edges->size());
	for(int i = 0; i < edges->size(); i++){
		oneForm.push_back(0.f);
	}
}

VectorField::~VectorField(void)
{

}



//sets the one form values associated to faceNr appropriately: projects dir in the
//pane spanned by the triangle
void VectorField::setOneForm(int faceNr, tuple3f & dir){
	vector<tuple3i> & fc2he = *(this->fc2he);
	vector<tuple3i> & faces = *(this->faces);
	vector<tuple3f> & vertices = *(this->vertices);
	vector<tuple2i> & hedges = * (this->edges);

	//vector orthogonal to halfedge ab, bc, ca
/*	tuple3f  p_ab, p_bc, p_ca;
	p_ab.set(vertices[faces[faceNr].b]);
	p_ab -= vertices[faces[faceNr].a];

	p_bc.set(vertices[faces[faceNr].c]);
	p_bc -= vertices[faces[faceNr].b];

	p_ca.set(vertices[faces[faceNr].a]);
	p_ca -= vertices[faces[faceNr].c];


	tuple3f normal = p_ab.cross(p_bc,p_ab);
	normal.normalize();

	tuple3f proj = dir - (normal*dir.dot(normal));  

//	std::cout <<"setting:" << proj.x << ", " << proj.y << ", " << proj.z << "\n";
//	std::cout <<"Lies in plane:" << proj.dot(normal)<< "\n";

	tuple2i edge;
	tuple3i face;
	int edgeID;
	face = faces[faceNr];
	edgeID = fc2he[faceNr].a;
	edge = hedges[edgeID];
	setOneForm(edgeID , face.orientation(edge), p_ab.dot(proj));

	edgeID = fc2he[faceNr].b;
	edge = hedges[edgeID];
	setOneForm(edgeID , face.orientation(edge), p_bc.dot(proj));

	edgeID = fc2he[faceNr].c;
	edge = hedges[edgeID];
	setOneForm(edgeID , face.orientation(edge), p_ca.dot(proj));*/
	tuple3i target_edg;
	tuple3f target_val;

	vectorFieldTools::vectorToOneForm(dir,faceNr,fc2he,hedges,myMesh,target_edg, target_val);

	this->oneForm[target_edg.a] = target_val.x;
	this->oneForm[target_edg.b] = target_val.y;
	this->oneForm[target_edg.c] = target_val.z;

}


float VectorField::getOneForm( int halfedge , int orientation)
{
//	return oneForm[abs(halfedge)] * sgn(halfedge);
	return oneForm[halfedge] * orientation;
}

void VectorField::setOneForm( int halfedge, int orientation, float val )
{
//	oneForm[abs(halfedge)] = sgn(halfedge) * val;
	oneForm[halfedge] = orientation * val;
}


//baricentric coordinates
tuple3f VectorField::oneForm2Vec(int faceNr, float bara, float barb, float barc){
	vector<tuple3i> & fc2he = *(this->fc2he);
	vector<tuple3i> & faces = *(this->faces);
	vector<tuple3f> & vertices = *(this->vertices);

	//vector orthogonal to halfedge ab, bc, ca
/*	tuple3f  p_abT, p_bcT, p_caT, normal;
	p_abT.set(vertices[faces[faceNr].b]);
	p_abT -= vertices[faces[faceNr].a];

	p_bcT.set(vertices[faces[faceNr].c]);
	p_bcT -= vertices[faces[faceNr].b];
	
	p_caT.set(vertices[faces[faceNr].a]);
	p_caT -= vertices[faces[faceNr].c];

	normal = - p_abT.cross(p_bcT);
	float vol2Triangle = normal.norm();
	normal.normalize();

	matrixf rot = matrixFactory::rotate(PI/2,normal.x, normal.y, normal.z);
	p_abT = rot*p_abT;
	p_bcT = rot*p_bcT;
	p_caT = rot*p_caT;

	tuple3i face = faces[faceNr];
	int edgeID1 = fc2he[faceNr].a; 
	int edgeID2 = fc2he[faceNr].b;
	tuple2i edge1 = (*edges)[edgeID1];
	tuple2i edge2 = (*edges)[edgeID2];
							//c_ab	alpha_a				//c_bc		alpha_c
	//tuple3f result = p_caT * (getOneForm(fc2he[faceNr].a)*bara - getOneForm(fc2he[faceNr].b)*barc) +
	tuple3f result = p_caT * (getOneForm(edgeID1, face.orientation(edge1))*bara - 
		getOneForm(edgeID2,face.orientation(edge2))*barc);
							//c_bc	alpha_b			//c_ca		alpha_a
	//result += p_abT * (getOneForm(fc2he[faceNr].b)*barb - getOneForm(fc2he[faceNr].c)*bara) +
	edgeID1 = fc2he[faceNr].b;
	edgeID2 = fc2he[faceNr].c;
	edge1 = (*edges)[edgeID1];
	edge2 = (*edges)[edgeID2];
	result += p_abT * (getOneForm(edgeID1,face.orientation(edge1))*barb 
		- getOneForm(edgeID2,face.orientation(edge2))*bara);
								//c_ca	alpha_c			//c_ab		alpha_b
//	result+= p_bcT * (getOneForm(fc2he[faceNr].c)*barc - getOneForm(fc2he[faceNr].a)*barb);
	edgeID1 = fc2he[faceNr].c;
	edgeID2 = fc2he[faceNr].a;
	edge1 = (*edges)[edgeID1];
	edge2 = (*edges)[edgeID2];
	result+= p_bcT * (getOneForm(edgeID1,face.orientation(edge1))*barc 
		- getOneForm(edgeID2,face.orientation(edge2))*barb);


	result *= 1.f/vol2Triangle;

//	std::cout << result.x << ", " << result.y << ", " << result.z << "\n";
//	std::cout << "Reconstruct lies in plane: " << result.dot(normal)<<"\n\n";*/

	tuple3f result;
	tuple3f oneFrm(oneForm[fc2he[faceNr].a],oneForm[fc2he[faceNr].b],oneForm[fc2he[faceNr].c]);
	vectorFieldTools::oneFormToVector(faces[faceNr],fc2he[faceNr],*edges, vertices,oneFrm,tuple3f(bara, barb, barc), result);

	return result;
}

void VectorField::glOutputField(bool normed, float displayLength){
	vector<tuple3i> & faces = *(this->faces);
	vector<tuple3f> & vertices = *(this->vertices);

	if(edges == NULL || this->oneForm.size() != edges->size()){
		return;
	}
	tuple3f pos ;
	tuple3f dir;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		
		pos = vertices[faces[i].a] + vertices[faces[i].b] + vertices[faces[i].c];
		pos *= 1.f/3;

		glColor3f(0,0,0);
		glBegin(GL_LINE_LOOP);
		glVertex3fv((GLfloat *) & pos);
		dir = oneForm2Vec(i,1.f/3,1.f/3,1.f/3);//*0.3f;
		if(normed){
			dir.normalize();
			pos+= dir*displayLength;
		}
		else{
			pos += dir*displayLength;
		}
		glColor3f(0,1,0);
		glVertex3fv((GLfloat *) & pos);
		glEnd();

	}
}
/*
void VectorField::setDisplayLength( double param1 )
{
	this->displayLength = (float) param1;
}*/
