#pragma once
#include "mesh.h"
#include "Observer.h"

class VectorField:Observer
{

public:
	VectorField(){}
	VectorField(mesh * aMesh);
	~VectorField(void);

	void update(void * src, int msg);
	void glOutputField();

private:
	//oneForm[i] is the value on the edge i
	vector<float> oneForm;

	//will set the oneForm value associated to the halfedge.
	//note: if halfedge is negative the convention is that this
	//means that the oposingly oriented halfedge is meant.
	//if negative the value will be stored appropriately on
	//the differently oriented halfedge (-1)
	void setOneForm(int halfedge, float val);
	//will get the oneForm value associated to the halfedge.
	//note: if halfedge is negative the convention is that this
	//means that the oposingly oriented halfedge is meant.
	//Here the negative of the stored value is returned
	float getOneForm(int halfedge);

	void setOneForm(int faceNr, tuple3f & dir);
	tuple3f oneForm2Vec(int faceNr, float bara, float barb, float barc);

	int abs(int a){
		return (a >=0?a:-a);
	}

	int sgn(int a){
		return (a >=0?1:-1);
	}
};
