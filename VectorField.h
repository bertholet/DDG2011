#pragma once
#include "mesh.h"
#include "tuple3.h"

class VectorField
{

public:
	VectorField(){
		fc2he = NULL;
		faces = NULL;
		vertices = NULL;
		edges = NULL;
	}
	VectorField(mesh * aMesh);
	VectorField(mesh * aMesh, tuple3f & dir);
	~VectorField(void);

//	void update(void * src, int msg);
	void glOutputField(bool normalizd);

	void setOneForm(int faceNr, tuple3f & dir);

	//will set the oneForm value associated to the oriented halfedge.
	//if orientation is negative the value will be stored appropriately on
	//the differently oriented halfedge (-1)
	//By defining the method like this it forces the user to think about the 
	//orientation
	void setOneForm(int halfedge, int orientation, float val);

private:
	//oneForm[i] is the value on the edge i
	vector<float> oneForm;
	vector<tuple3i> * fc2he;
	vector<tuple3i> * faces;
	vector<tuple3f> *vertices;
	vector<tuple2i> *edges;
	mesh * myMesh;

	//will get the oneForm value associated to the halfedge.
	//note: if halfedge is negative the convention is that this
	//means that the oposingly oriented halfedge is meant.
	//Here the negative of the stored value is returned
	float getOneForm(int halfedge, int orientation);

	tuple3f oneForm2Vec(int faceNr, float bara, float barb, float barc);

/*	int abs(int a){
		return (a >=0?a:-a);
	}

	int sgn(int a){
		return (a >=0?1:-1);
	}*/
};
