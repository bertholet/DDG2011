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



		flux2vel.setRow(0, (b-a).cross(n));
		flux2vel.setRow(1, (c-b).cross(n));
		flux2vel.setRow(2,n);

		flux2vel = flux2vel.inv();

		//flux should sum to 0. 
		flx.x = flux.get(f2e[i].a, fcs[i].orientation(edgs[f2e[i].a]));
		flx.y = flux.get(f2e[i].b, fcs[i].orientation(edgs[f2e[i].b]));
		flx.z = 0;


		target[i].set(flux2vel * flx);
	}
}


float fluidTools::bariWeight( tuple3f & point ,int nr, int dualFace_id, std::vector<int> & dualVert_ids, std::vector<tuple3f> & dualVert_pos, meshMetaInfo & mesh )
{
	std::vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	tuple3i & face = mesh.getBasicMesh().getFaces()[dualVert_ids[nr]];
	tuple3f & pos = point; 
	tuple3f & vert = dualVert_pos[dualVert_ids[nr]];
	int other1 = face.a;
	int other2 = face.b;
	if (other1 == dualFace_id){
		other1 = face.b;
		other2 = face.c;
	}
	else if(other2 == dualFace_id){
		other2 = face.c;
	}

	//Actually only true in plane. a+b/...
	tuple3f n1 = verts[other1] - verts[dualFace_id];
	n1.normalize();
	tuple3f n2 = verts[other2] -verts[dualFace_id];
	n2.normalize();
	return (n1.cross(n2)).norm() / (nonzeroDot(n1, pos-vert)*nonzeroDot(n2, pos-vert));
}

float fluidTools::nonzeroDot( tuple3f & n, tuple3f & pos )
{
	float dot = n.dot(pos);
	if(dot>=0){
		dot+= 10E-10;
	}
	else{
		dot-= 10E-10;//0.0000001f;
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
		tmp = (tmp<0? -tmp:tmp);
		assert(tmp*0 == 0 );
		target.push_back(tmp);
		sum += tmp;
	}

	for(int i = 0; i < nrFcs; i++){
		target[i]/=sum;
	}

}

//////////////////////////////////////////////////////////////////////////
// dualVertPos, the circumcenters as can be calculated with dualMeshTools
//////////////////////////////////////////////////////////////////////////
void fluidTools::dirs2Flux( std::vector<tuple3f> & dirs, oneForm & target, meshMetaInfo & mesh /*, vector<tuple3f> & dualVert*/)
{
	assert(target.getMesh() == &mesh);
	vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	vector<tuple3i> & fcs = mesh.getBasicMesh().getFaces();
	vector<tuple3i> & f2e = * mesh.getFace2Halfedges();
	vector<tuple2i> & edges = * mesh.getHalfedges();

	tuple3i fc,he;
	tuple3f n_edge;
	tuple2i edge;
	float old_val;

	//for dbg
	tuple3f n;
	tuple3f n_ab, n_bc, n_ca;

	//reset the oneForm
	target.setZero();

	for(int i = 0; i < fcs.size();i++){

		//for dbg
		tuple3f & a = verts[fcs[i].a];
		tuple3f & b = verts[fcs[i].b];
		tuple3f & c = verts[fcs[i].c];
		n = (b-a).cross(c-a);
		n.normalize();
		//normals
		n_ab = (b-a).cross(n);
		n_bc = (c-b).cross(n);
		n_ca = (a-c).cross(n);
		//gbd rof

		fc = fcs[i];
		he = f2e[i];
	/*	n_edge = ((verts[fc.a]+verts[fc.b])*0.5f - dualVert[i]);
		n_edge.normalize();
		n_edge *= ((verts[fc.b]-verts[fc.a]).norm());*/
		
		old_val = target.get(he.a,fc.orientation(edges[he.a]));
		n_edge.set(n_ab);
		if(old_val==0){
			target.set(he.a, n_edge.dot(dirs[i]) /*((verts[fc.b]-verts[fc.a]).norm())*/ , fc.orientation(edges[he.a]));
		}
		else{
			target.set(he.a, (n_edge.dot(dirs[i])/*((verts[fc.b]-verts[fc.a]).norm())*/ + old_val)/2, fc.orientation(edges[he.a]));
		}


/*		n_edge = ((verts[fc.b]+verts[fc.c])*0.5f - dualVert[i]);
		n_edge.normalize();*/
		n_edge.set(n_bc);
		old_val = target.get(he.b,fc.orientation(edges[he.b]));
		if(old_val==0){
			target.set(he.b, n_edge.dot(dirs[i]) /* ((verts[fc.c]-verts[fc.b]).norm())*/, fc.orientation(edges[he.b]));
		}
		else{
			target.set(he.b, (n_edge.dot(dirs[i]) /* ((verts[fc.c]-verts[fc.b]).norm())*/+old_val)/2, fc.orientation(edges[he.b]));
		}
	

	/*	n_edge = ((verts[fc.a]+verts[fc.c])*0.5f - dualVert[i]);
		n_edge.normalize();*/
		n_edge.set(n_ca);
		old_val = target.get(he.c,fc.orientation(edges[he.c]));
		if(old_val==0){
			target.set(he.c, n_edge.dot(dirs[i]) /* ((verts[fc.c]-verts[fc.a]).norm())*/, fc.orientation(edges[he.c]));
		}
		else{
			target.set(he.c, (n_edge.dot(dirs[i]) /* ((verts[fc.c]-verts[fc.a]).norm())*/ + old_val)/2, fc.orientation(edges[he.c]));
		}

	}
}

void fluidTools::flux2Vorticity( oneForm & flux, nullForm & vorticity, meshMetaInfo & myMesh , pardisoMatrix & dt_star1)
{
	dt_star1.mult(flux.getVals(), vorticity.getVals());
}









