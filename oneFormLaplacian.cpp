#include "oneFormLaplacian.h"
#include "meshOperation.h"
#include <algorithm>
#include <assert.h>
#include "Operator.h"

using namespace std;

oneFormLaplacian::oneFormLaplacian( vector<tuple3i> * faces2he, 
		vector<tuple2i> * edges, 
		mesh * amesh)
{
	this->fc2he = faces2he;
	this->edges = edges;
	this->v2nbrf = &(amesh->getNeighborFaces());
	this->faces = &(amesh->getFaces());
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
float oneFormLaplacian::valdd( int edgij , int edgj )
{
	tuple2i e_ij = (*edges)[edgij];
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
		result += face2.orientation(e_ij) * face2.orientation(e_other) *
			1.f/Operator::area(fc2,*myMesh) ;
	}

	return result;
}

float oneFormLaplacian::val_deltadelta( int edgij, int edgother )
{
	//if other contains j: + ....
	tuple2i e_ij = (*edges)[edgij];
	tuple2i e_other = (*edges)[edgother];

	int j = e_ij.b, i = e_ij.a;
	float result = 0;

	//plus
	if(e_other.a == j){
		result += 1.f/ Operator::aVornoi(j, *myMesh) * Operator::dualEdge_edge_ratio(e_ij.a,e_ij.b,*myMesh) * 
			Operator::dualEdge_edge_ratio(e_other.a,e_other.b, *myMesh);
	}
	//minus, because of orientation
	else if (e_other.b ==j){
		result -= 1.f/ Operator::aVornoi(j, *myMesh) * Operator::dualEdge_edge_ratio(e_ij.a,e_ij.b,*myMesh) * 
			Operator::dualEdge_edge_ratio(e_other.a,e_other.b, *myMesh);
	}

	//note: the following two ifs can not be incorporated in the aboves because different vornoi area.
	//minus
	if(e_other.a == i){
		result -= 1.f/ Operator::aVornoi(i, *myMesh) * Operator::dualEdge_edge_ratio(e_ij.a,e_ij.b,*myMesh) * 
			Operator::dualEdge_edge_ratio(e_other.a,e_other.b, *myMesh);
	}
	//plus, because of orientation
	else if (e_other.b == i){
		result += 1.f/ Operator::aVornoi(i, *myMesh) * Operator::dualEdge_edge_ratio(e_ij.a,e_ij.b,*myMesh) * 
			Operator::dualEdge_edge_ratio(e_other.a,e_other.b, *myMesh);
	}

	return result;
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
	int lk = 0;
}

void oneFormLaplacian::stard( vector<int> & srcsink_verts, vector<float> & constr , double * target, int sz)
{
	assert(sz == edges->size());
	vector<int> nbrEdges;
	vector<int>::iterator nbr_edge;
	tuple2i edge;
	int vert;
	for(int i = 0; i < srcsink_verts.size(); i++){
		vert = srcsink_verts[i];
	//for(vector<int>::iterator vert = srcsink_verts.begin(); vert!= srcsink_verts.end(); vert++){
		meshOperation::getNeighborEdges(vert, *v2nbrf, *fc2he, *edges, nbrEdges);

		for(nbr_edge = nbrEdges.begin(); nbr_edge != nbrEdges.end(); nbr_edge++){
			assert(*nbr_edge< sz);
			edge = (*edges)[*nbr_edge];
			target[*nbr_edge] += edge.orientation(vert) * constr[i];
				Operator::dualEdge_edge_ratio(edge.a,edge.b, *myMesh);
		}
	}
		
}
