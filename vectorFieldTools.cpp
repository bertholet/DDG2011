#include "vectorFieldTools.h"

vectorFieldTools::vectorFieldTools(void)
{
}

vectorFieldTools::~vectorFieldTools(void)
{
}

void vectorFieldTools::vectorToOneForm( tuple3f & dir, 
		int faceNr, vector<tuple3i> & fc2e, 
		vector<tuple2i> & edges, 
		mesh * m, 
		tuple3i & target_edges, 
		tuple3f & target_vals )
{
	vector<tuple3i> & faces = m->getFaces();
	vector<tuple3f> & vertices = m->getVertices();

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

	tuple2i edge;
	tuple3i face;
	int edgeID;
	face = faces[faceNr];
	edgeID = fc2e[faceNr].a;
	edge = edges[edgeID];
	target_edges.a = edgeID;
	target_vals.x = face.orientation(edge) * p_ab.dot(proj);

	edgeID = fc2e[faceNr].b;
	edge = edges[edgeID];
	target_edges.b = edgeID;
	target_vals.y = face.orientation(edge) * p_bc.dot(proj);

	edgeID = fc2e[faceNr].c;
	edge = edges[edgeID];
	target_edges.c = edgeID;
	target_vals.z = face.orientation(edge) * p_ca.dot(proj);

}

void vectorFieldTools::oneFormToVector()
{
	assert(false);
	throw runtime_error("not implemented!!!!!");
}
