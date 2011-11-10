#include "StdAfx.h"
#include "meshOperation.h"
#include <assert.h>

meshOperation::meshOperation(void)
{
}

meshOperation::~meshOperation(void)
{
}

int meshOperation::getPrevious( int center_index, int v, mesh  & m)
{
	vector<tuple3i> & faces = m.getFaces(); 
	vector<int> & neighbor_faces = m.getNeighborFaces()[center_index];
	tuple3i face;
	
	for(vector<int>::iterator it = neighbor_faces.begin(); it != neighbor_faces.end(); it++){
		face = faces[*it];
		if(face.a == v && face.b == center_index){
			return face.c;
		}
		if(face.b == v && face.c == center_index){
			return face.a;
		}
		if(face.c == v && face.a == center_index){
			return face.b;
		}
	}


	return -1;
}

int meshOperation::getNext( int center_idx, int v, mesh  & m )
{
	vector<tuple3i> & faces = m.getFaces(); 
	vector<int> & neighbor_faces = m.getNeighborFaces()[center_idx];
	tuple3i  face;

	for(vector<int>::iterator it = neighbor_faces.begin(); it != neighbor_faces.end(); it++){
		face = faces[*it];
		if(face.a == v && face.c == center_idx){
			return face.b;
		}
		if(face.b == v && face.a == center_idx){
			return face.c;
		}
		if(face.c == v && face.b == center_idx){
			return face.a;
		}
	}

	return -1;
}

float meshOperation::sumAnglesWheel( int from, int center, int to, mesh & m )
{
	vector<int> & nbrs = m.getNeighbors()[center];
	vector<int> & nbr_fcs = m.getNeighborFaces()[center];


	int actual = from, next, lps = 0;
	float angle = 0;

	do{
		next = meshOperation::getPrevious(center,actual, m);
		if(next < 0 || lps > int(nbrs.size())){
			throw std::runtime_error("Assertion failed at sum Angle Wheel");
		}
		angle += tuple3f::angle( m.vertices[actual], m.vertices[center], m.vertices[next]);
		actual = next;
		lps++;
	}
	while (actual != to);

	return angle;
}

int meshOperation::getFirst( int center_index, int v, mesh& m )
{
	int actual = v, previous;
	while((previous =getPrevious(center_index,actual,m))>=0){
		actual = previous;
	}
	return actual;
}

int meshOperation::getLast( int center_index, int v, mesh& m )
{
	int actual = v, next;
	while((next =getNext(center_index,actual,m))>=0){
		actual = next;
	}
	return actual;
}

int meshOperation::getPrevious_bc( int center_index, int v, mesh& m )
{
	int prev = getPrevious(center_index,v,m);
	if(prev < 0){
		return getLast(center_index,v,m);
	}
	return prev;
}

int meshOperation::getNext_bc( int center_index, int v, mesh& m )
{

	int next = getNext(center_index,v,m);
	if(next < 0){
		return getFirst(center_index,v,m);
	}
	return next;
}

// p.x-> 2x-p.x
void meshOperation::mirrorX( std::vector<tuple3f> & borderPos, float x )
{
	for(unsigned int i = 0; i < borderPos.size(); i++){
		borderPos[i].x = 2*x-borderPos[i].x;
	}
}


void meshOperation::getOrientedEdges( mesh & m, vector<tuple3i> & target_fc_halfEdges, vector<tuple2i> & target_halfEdges )
{
	vector<tuple3i> & faces = m.getFaces();
	vector<tuple3i>::iterator it;
	vector<tuple2i>::iterator el;
	target_halfEdges.clear();
	target_fc_halfEdges.clear();

	tuple2i halfedge;
	int sign;
	for(it = faces.begin(); it!= faces.end(); it++){

		halfedge.set((it->a < it->b ? (*it).a: (*it).b),
			(it->a < it->b ? (*it).b: (*it).a));
		el = lower_bound(target_halfEdges.begin(),target_halfEdges.end(), halfedge);
		target_halfEdges.insert(el,halfedge);

		//halfedge.set((*it).b, (*it).c);
		halfedge.set((it->b < it->c ? (*it).b: (*it).c),
			(it->b < it->c ? (*it).c: (*it).b));
		el = lower_bound(target_halfEdges.begin(),target_halfEdges.end(), halfedge);
		target_halfEdges.insert(el,halfedge);

		//halfedge.set((*it).c, (*it).a);
		halfedge.set((it->a < it->c ? (*it).a: (*it).c),
			(it->a < it->c ? (*it).c: (*it).a));
		el = lower_bound(target_halfEdges.begin(),target_halfEdges.end(), halfedge);
		target_halfEdges.insert(el,halfedge);
	}

	tuple3i halfedge_index;
	for(it = faces.begin(); it!= faces.end(); it++){
		//halfedge.set(it->a, it->b);
		sign = (it->a < it->b ? 1: -1);
		halfedge.set((it->a < it->b ? (*it).a: (*it).b),
			(it->a < it->b ? (*it).b: (*it).a));
		el = lower_bound(target_halfEdges.begin(),target_halfEdges.end(), halfedge);
		halfedge_index.a = sign * (el - target_halfEdges.begin());

		//halfedge.set(it->b, it->c);
		sign = (it->b < it->c ? 1: -1);
		halfedge.set((it->b < it->c ? (*it).b: (*it).c),
			(it->b < it->c ? (*it).c: (*it).b));
		el = lower_bound(target_halfEdges.begin(),target_halfEdges.end(), halfedge);
		halfedge_index.b = sign*(el - target_halfEdges.begin());

		//halfedge.set(it->c, it->a);
		sign = (it->c < it->a ? 1: -1);
		halfedge.set((it->a < it->c ? (*it).a: (*it).c),
			(it->a < it->c ? (*it).c: (*it).a));
		el = lower_bound(target_halfEdges.begin(),target_halfEdges.end(), halfedge);
		halfedge_index.c = sign*(el - target_halfEdges.begin());
		target_fc_halfEdges.push_back(halfedge_index);
	}
}

void meshOperation::getHalf( mesh & m, mesh & target, tuple3f direction, float dist )
{
	vector<int> usedVertices;
	vector<tuple3i> faces;
	vector<tuple3f> vertices;

	for(unsigned int i = 0; i < m.faces.size(); i++){

		if(((tuple3f) m.vertices[m.faces[i].a]).dot(direction)>dist ||
			((tuple3f) m.vertices[m.faces[i].b]).dot(direction)>dist||
			((tuple3f) m.vertices[m.faces[i].c]).dot(direction)>dist){
				usedVertices.push_back(m.faces[i].a);
				usedVertices.push_back(m.faces[i].b);
				usedVertices.push_back(m.faces[i].c);

				faces.push_back(m.faces[i]);
		}
	}

	vector<int>::iterator it;

	// using default comparison (operator <):
	std::sort (usedVertices.begin(), usedVertices.end());
	usedVertices.erase(std::unique(usedVertices.begin(), usedVertices.end()), usedVertices.end());

	int k;
	for(unsigned int i = 0; i < faces.size(); i++){
		k= (std::find(usedVertices.begin(), usedVertices.end(), faces[i].a)-usedVertices.begin());
		faces[i].a = k;
		k= (std::find(usedVertices.begin(), usedVertices.end(), faces[i].b)-usedVertices.begin());
		faces[i].b = k;
		k= (std::find(usedVertices.begin(), usedVertices.end(), faces[i].c)-usedVertices.begin());
		faces[i].c = k;

	}

	for(unsigned int i = 0; i < usedVertices.size(); i++){
		vertices.push_back(m.vertices[usedVertices[i]]);
	}

	target.reset(vertices,faces);
}

void meshOperation::getNbrFaces( tuple2i & edge, int * fc1, int * fc2, 
		vector<vector<int>> & vertex2Face)
{
	vector<int> & fc_a = vertex2Face[edge.a];
	vector<int> & fc_b = vertex2Face[edge.b];

	vector<int>::iterator it;
	(*fc1) = -1;
	(*fc2) = -1;
	for(int i = 0; i < fc_a.size(); i++){
		it = find(fc_b.begin(), fc_b.end(), fc_a[i]);
		if(*it == fc_a[i]){
			if(*fc1 == -1){
				(*fc1) = *it;
			}
			else if ( *fc2 == -1){
				(*fc2) = *it;
			}
			else{
				assert(false);
			}
		}
	}

}

/*int meshOperation::orientation( tuple2i & edge, tuple3i & face )
{
	assert(edge.a == face.a || edge.a == face.b || edge.a == face.c);
	assert(edge.b == face.a || edge.b == face.b || edge.b == face.c);
	
	if(face.a == edge.a && face.b == edge.b){
		return 1;
	}
	if(face.b == edge.a && face.c == edge.b){
		return 1;
	}
	if(face.c == edge.a && face.a == edge.b){
		return 1;
	}
	return -1;
}*/


