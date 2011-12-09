#include "vectorFieldTools.h"
#include "matrixFactory.h"
#include "stdafx.h"

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


	tuple3f normal = p_bc.cross(p_ab);
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
	target_vals.x =  face.orientation(edge) * p_ab.dot(proj);

	edgeID = fc2e[faceNr].b;
	edge = edges[edgeID];
	target_edges.b = edgeID;
	target_vals.y =  face.orientation(edge) * p_bc.dot(proj);

	edgeID = fc2e[faceNr].c;
	edge = edges[edgeID];
	target_edges.c = edgeID;
	target_vals.z =  face.orientation(edge) * p_ca.dot(proj);

}

void vectorFieldTools::oneFormToVector(tuple3i & face,
		   tuple3i & f2e,
			vector<tuple2i> & edges,
			vector<tuple3f> & verts,
		   tuple3f & oneForm, 
		   tuple3f & bari_coord,
		   tuple3f & target_dir)
{

	//vector orthogonal to halfedge ab, bc, ca
	tuple3f  p_abT, p_bcT, p_caT, normal;
	p_abT.set(verts[face.b]);
	p_abT -= verts[face.a];

	p_bcT.set(verts[face.c]);
	p_bcT -= verts[face.b];

	p_caT.set(verts[face.a]);
	p_caT -= verts[face.c];

	normal = - p_abT.cross(p_bcT);
	float vol2Triangle = normal.norm();
	normal.normalize();

	matrixf rot = matrixFactory::rotate(PI/2,normal.x, normal.y, normal.z);
	p_abT = rot*p_abT;
	p_bcT = rot*p_bcT;
	p_caT = rot*p_caT;

	tuple2i edge1 = (edges)[f2e.a];
	tuple2i edge2 = (edges)[f2e.b];
	//c_ab	alpha_a				//c_bc		alpha_c
	//tuple3f result = p_caT * (getOneForm(fc2he[faceNr].a)*bara - getOneForm(fc2he[faceNr].b)*barc) +
	tuple3f result = p_caT * (oneForm.x* face.orientation(edge1)*bari_coord.x - 
		oneForm.y * face.orientation(edge2)*bari_coord.z);
	//c_bc	alpha_b			//c_ca		alpha_a
	//result += p_abT * (getOneForm(fc2he[faceNr].b)*barb - getOneForm(fc2he[faceNr].c)*bara) +
	
	edge1 = (edges)[f2e.b];
	edge2 = (edges)[f2e.c];
	result += p_abT * (oneForm.y * face.orientation(edge1)*bari_coord.y 
		- oneForm.z * face.orientation(edge2)*bari_coord.x);
	//c_ca	alpha_c			//c_ab		alpha_b
	//	result+= p_bcT * (getOneForm(fc2he[faceNr].c)*barc - getOneForm(fc2he[faceNr].a)*barb);
	
	edge1 = (edges)[f2e.c];
	edge2 = (edges)[f2e.a];
	result+= p_bcT * (oneForm.z * face.orientation(edge1)*bari_coord.z
		- oneForm.x  * face.orientation(edge2)*bari_coord.y);


	result *= 1.f/vol2Triangle;

	//	std::cout << result.x << ", " << result.y << ", " << result.z << "\n";
	//	std::cout << "Reconstruct lies in plane: " << result.dot(normal)<<"\n\n";

	target_dir.set(result);
}
