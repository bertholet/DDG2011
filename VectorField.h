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

	void setOneForm(int faceNr, tuple3f & dir);
	tuple3f oneForm2Vec(int faceNr, float bara, float barb, float barc);
};
