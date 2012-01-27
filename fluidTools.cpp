#include "fluidTools.h"
#include "matrixf.h"

fluidTools::fluidTools(void)
{
}

fluidTools::~fluidTools(void)
{
}

void fluidTools::flux2Velocity( oneForm & flux, std::vector<tuple3f> & target, meshMetaInfo & mesh)
{
	std::vector<tuple3i> & fcs = mesh.getBasicMesh().getFaces();
	std::vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	std::vector<tuple3i> & f2e = * mesh.getFace2Halfedges();
	std::vector<tuple2i> & edgs = * mesh.getHalfedges();

	if(target.size() != fcs.size()){
		target.clear();
		for(int i= 0; i < fcs.size(); i++){
			target.push_back(tuple3f(0,0,0));
		}
	}
	assert(target.size() == fcs.size());

	matrix3f flux2vel;
	matrix3f n_cross_T;
	//triangle normal
	tuple3f n;
	//flux of actual face
	tuple3f flx;

	for(int i = 0; i < fcs.size(); i++){
		tuple3f & a = verts[fcs[i].a];
		tuple3f & b = verts[fcs[i].b];
		tuple3f & c = verts[fcs[i].c];

		n = (b-a).cross(c-a);
		n.normalize();

// 		//not full rank.
// 		flux2vel.set(b.x-a.x,b.y-a.y,b.z-a.z,
// 			c.x-b.x,c.y-b.y,c.z-b.z,
// 			a.x-c.x,a.y-c.y,a.z-c.z);	
// 		//not full rank
// 		n_cross_T.set(0,n.z,-n.y,
// 					-n.z,0,n.x,
// 					n.y,-n.x,0);
// 
// 		flux2vel = (flux2vel*n_cross_T);

		flux2vel.setRow(0, (b-a).cross(n));
		flux2vel.setRow(1, (c-b).cross(n));
		flux2vel.setRow(2,n);

		flux2vel = flux2vel.inv();

		//flux should sum to 0. 
		flx.x = flux.get(f2e[i].a,fcs[i].orientation(edgs[f2e[i].a]));
		flx.y = flux.get(f2e[i].b, fcs[i].orientation(edgs[f2e[i].b]));
		flx.z = 0;


		target[i].set(flux2vel * flx);
	}
}


float fluidTools::bariWeight( tuple3f & point ,int nr, int dualFace_id, std::vector<int> & dualVert_ids, std::vector<tuple3f> & dualVert_pos, meshMetaInfo & mesh )
{
	std::vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	tuple3i & face = mesh.getBasicMesh().getFaces()[dualVert_ids[nr]];
	tuple3f & pos = point; // shit: dualVert_pos[dualVert_ids[nr]];
	int other1 = face.a;
	int other2 = face.b;
	if (other1 == dualFace_id){
		other1 = face.b;
		other2 = face.c;
	}
	else if(other2 == dualFace_id){
		other2 = face.c;
	}

	//false!!!!!!!!!! only true in plane. a+b/...
	tuple3f n1 = verts[other1] - verts[dualFace_id];
	n1.normalize();
	tuple3f n2 = verts[other2] -verts[dualFace_id];
	n2.normalize();
	return (n1.cross(n2)).norm() / (nonzeroDot(n1, pos)*nonzeroDot(n2, pos));
}

float fluidTools::nonzeroDot( tuple3f & n, tuple3f & pos )
{
	float dot = n.dot(pos);
	if(dot>=0){
		dot+= 0.0000001f;
	}
	else{
		dot-= 0.0000001f;
	}
	return dot;
}

void fluidTools::bariCoords( tuple3f & point, int dualFace_id, std::vector<tuple3f> & dualVert_pos, 
							std::vector<float> & target, meshMetaInfo & mesh )
{
	std::vector<int> & v2f = mesh.getBasicMesh().getNeighborFaces()[dualFace_id];
	int nrFcs = v2f.size();
	target.clear();
	target.reserve(nrFcs);
	float sum = 0;
	float tmp;
	for(int i = 0; i<nrFcs; i++){
		tmp = bariWeight(point, i,dualFace_id,v2f,dualVert_pos,mesh);
		target.push_back(tmp);
		sum += tmp;
	}

	for(int i = 0; i < nrFcs; i++){
		target[i]/=sum;
	}

}


void fluidTools::vorticity2flux( twoForm & vorticity, oneForm & flux )
{

}






