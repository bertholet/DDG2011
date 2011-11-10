#include "oneFormLaplacian.h"
#include "meshOperation.h"
#include <algorithm>
#include <assert.h>
#include "vectorFieldTools.h"
#include "Operator.h"

using namespace std;

oneFormLaplacian::oneFormLaplacian( vector<tuple3i> * faces2he, 
		vector<tuple2i> * edges, 
		vector<vector<int>> * nbr_fcs,
		vector<tuple3i> * faces,
		mesh * amesh)
{
	this->fc2he = faces2he;
	this->edges = edges;
	this->v2nbrf = nbr_fcs;
	this->faces = faces;
	this->myMesh = amesh;
}

oneFormLaplacian::~oneFormLaplacian(void)
{
}

float oneFormLaplacian::val( int i , int j )
{
	return valdd(i,j) + val_deltadelta(i,j);
}

//weight of edgej for laplacian at edgei
float oneFormLaplacian::valdd( int edgi , int edgj )
{
	tuple2i e_ij = (*edges)[edgi];
	tuple2i e_other = (*edges)[edgj];

	int fc1, fc2;
	meshOperation::getNbrFaces(e_ij,&fc1, &fc2, *(this->v2nbrf));
	assert(fc2 >=0);

	tuple3i & face1 = (*faces)[fc1];
	tuple3i & face2 = (*faces)[fc2];
	
	float result=0;

	//if is border of face 1 add sgn* /face one
	//if is border of face 2 add sgn* /face two

	if(face1.contains(e_other)){
		result += face1.orientation(e_ij) * face1.orientation(e_other) *
			1.f/Operator::area(fc1,*myMesh) ;
	}

	if(face2.contains(e_other)){
		result += face1.orientation(e_ij) * face1.orientation(e_other) *
			1.f/Operator::area(fc2,*myMesh) ;
	}

	return result;
}

float oneFormLaplacian::val_deltadelta( int i, int j )
{
	return 0;
}
void oneFormLaplacian::indices( int edg, vector<int> & target )
{
	target.clear();
	tuple2i hedge = (*edges)[abs(edg)];
	int i = hedge.a;
	int j = hedge.b;

	vector<int>::iterator face = (*v2nbrf)[i].begin(),
		end = (*v2nbrf)[i].end();

	tuple3i edgeIDs;
	for(;face != end; face++){
		edgeIDs = (*fc2he)[*face];

		hedge = (*edges)[abs(edgeIDs.a)];
		if(hedge.a == i || hedge.b == i){
			target.push_back(abs(edgeIDs.a));
		}

		hedge = (*edges)[abs(edgeIDs.b)];
		if(hedge.a == i || hedge.b == i){
			target.push_back(abs(edgeIDs.b));
		}

		hedge = (*edges)[abs(edgeIDs.c)];
		if(hedge.a == i || hedge.b == i){
			target.push_back(abs(edgeIDs.c));
		}
	}

	face = (*v2nbrf)[j].begin();
	end = (*v2nbrf)[j].end();

	for(;face != end; face++){
		edgeIDs = (*fc2he)[*face];

		hedge = (*edges)[abs(edgeIDs.a)];
		if(hedge.a == j || hedge.b == j){
			target.push_back(abs(edgeIDs.a));
		}

		hedge = (*edges)[abs(edgeIDs.b)];
		if(hedge.a == j || hedge.b == j){
			target.push_back(abs(edgeIDs.b));
		}

		hedge = (*edges)[abs(edgeIDs.c)];
		if(hedge.a == j || hedge.b == j){
			target.push_back(abs(edgeIDs.c));
		}
	}
	
	std::sort(target.begin(), target.end());
	target.erase(std::unique(target.begin(), target.end()), target.end());
}
