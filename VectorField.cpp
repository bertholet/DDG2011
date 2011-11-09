#include "VectorField.h"
#include "Model.h"
#include "meshMetaInfo.h"
#include <vector>
#include "tuple3.h"
#include "matrixFactory.h"
#include "matrixf.h"
#include "stdafx.h"
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>

VectorField::VectorField( mesh * aMesh )
{
	aMesh->attach(this);
	meshMetaInfo * info = Model::getModel()->getMeshInfo();
	//info->
	vector<tuple2i> * edges = info->getHalfedges();
	oneForm.reserve(edges->size());
	for(int i = 0; i < edges->size(); i++){
		oneForm.push_back(0.f);
	}

	for(int i = 0; i < aMesh->faces.size(); i++){
		setOneForm(i,tuple3f(0,0,1));
	}

	cout << ";";
}

VectorField::~VectorField(void)
{

}

void VectorField::update( void * src, int msg )
{

}

//sets the one form values associated to faceNr appropriately: projects dir in the
//pane spanned by the triangle
void VectorField::setOneForm(int faceNr, tuple3f & dir){
	vector<tuple3i> & fc2he = *(Model::getModel()->getMeshInfo()->getFace2Halfedges());
	vector<tuple3i> & faces = Model::getModel()->getMesh()->getFaces();
	vector<tuple3f> & vertices = Model::getModel()->getMesh()->getVertices();
	vector<tuple2i> & hedges = * Model::getModel()->getMeshInfo()->getHalfedges();

	//vector orthogonal to halfedge ab, bc, ca
	tuple3f  p_ab, p_bc, p_ca;
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

	setOneForm(fc2he[faceNr].a , p_ab.dot(proj));
	setOneForm(fc2he[faceNr].b , p_bc.dot(proj));
	setOneForm(fc2he[faceNr].c , p_ca.dot(proj));
	/*oneForm[abs(fc2he[faceNr].a)] = sgn(fc2he[faceNr].a) * p_ab.dot(proj);
	oneForm[abs(fc2he[faceNr].b)] = sgn((fc2he[faceNr].b)) * p_bc.dot(proj);
	oneForm[abs(fc2he[faceNr].c)] = sgn(fc2he[faceNr].c) * p_ca.dot(proj);*/
}


float VectorField::getOneForm( int halfedge )
{
	return oneForm[abs(halfedge)] * sgn(halfedge);
}

void VectorField::setOneForm( int halfedge, float val )
{
	oneForm[abs(halfedge)] = sgn(halfedge) * val;
}


//baricentric coordinates
tuple3f VectorField::oneForm2Vec(int faceNr, float bara, float barb, float barc){
	vector<tuple3i> & fc2he = *(Model::getModel()->getMeshInfo()->getFace2Halfedges());
	vector<tuple3i> & faces = Model::getModel()->getMesh()->getFaces();
	vector<tuple3f> & vertices = Model::getModel()->getMesh()->getVertices();

	//vector orthogonal to halfedge ab, bc, ca
	tuple3f  p_abT, p_bcT, p_caT, normal;
	p_abT.set(vertices[faces[faceNr].b]);
	p_abT -= vertices[faces[faceNr].a];

	p_bcT.set(vertices[faces[faceNr].c]);
	p_bcT -= vertices[faces[faceNr].b];
	
	p_caT.set(vertices[faces[faceNr].a]);
	p_caT -= vertices[faces[faceNr].c];

	normal = p_abT.cross(p_abT,p_bcT);
	float vol2Triangle = normal.norm();
	normal.normalize();

	matrixf rot = matrixFactory::rotate(PI/2,normal.x, normal.y, normal.z);
	p_abT = rot*p_abT;
	p_bcT = rot*p_bcT;
	p_caT = rot*p_caT;


							//c_ab	alpha_a				//c_bc		alpha_c
	tuple3f result = p_caT * (getOneForm(fc2he[faceNr].a)*bara - getOneForm(fc2he[faceNr].b)*barc) +
								//c_bc	alpha_b			//c_ca		alpha_a
		p_abT * (getOneForm(fc2he[faceNr].b)*barb - getOneForm(fc2he[faceNr].c)*bara) +
								//c_ca	alpha_c			//c_ab		alpha_b
		p_bcT * (getOneForm(fc2he[faceNr].c)*barc - getOneForm(fc2he[faceNr].a)*barb);
	result *= 1.f/vol2Triangle;

//	std::cout << result.x << ", " << result.y << ", " << result.z << "\n";
//	std::cout << "Reconstruct lies in plane: " << result.dot(normal)<<"\n\n";

	return result;
}

void VectorField::glOutputField(){
	vector<tuple3i> & faces = Model::getModel()->getMesh()->getFaces();
	vector<tuple3f> & vertices = Model::getModel()->getMesh()->getVertices();
	vector<tuple2i> * hedges = Model::getModel()->getMeshInfo()->getHalfedges();
	if(this->oneForm.size() != hedges->size()){
		return;
	}
	tuple3f pos ;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		
		pos = vertices[faces[i].a] + vertices[faces[i].b] + vertices[faces[i].c];
		pos *= 1.f/3;

		glColor3f(0,1,0);
		glBegin(GL_LINE_LOOP);
		glVertex3fv((GLfloat *) & pos);
		pos+= oneForm2Vec(i,1.f/3,1.f/3,1.f/3);
		glVertex3fv((GLfloat *) & pos);
		glEnd();

	}
}
