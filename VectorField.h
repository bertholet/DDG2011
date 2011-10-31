#pragma once
#include "mesh.h"
#include "Observer.h"

class VectorField:Observer
{

	//edgestructure: Faces To halfedges
	//points to Halfedges 
public:
	VectorField(mesh * aMesh);
	~VectorField(void);

	void update(void * src, int msg);
};
