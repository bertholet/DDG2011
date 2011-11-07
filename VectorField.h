#pragma once
#include "mesh.h"
#include "Observer.h"

class VectorField:Observer
{

	//edgestructure: Faces To halfedges i.e. vector<tuple3i> fc_halfedges sucht that
	//fc_halfedge(face) = tuple3i with indices of haledges
	//Halfedges to points. the actual halfedges. list of integer pairs.
	//Halfedge(i) = (a,b) = ith halfedge.
	//put halfedges and fc_halfedges into 
	//nrbs of vertices ~= vertex to halfedge.
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
