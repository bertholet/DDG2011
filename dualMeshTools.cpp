#include "dualMeshTools.h"
#include "tuple3.h"

dualMeshTools::dualMeshTools(void)
{
}

dualMeshTools::~dualMeshTools(void)
{
}


void dualMeshTools::getDualVertices( meshMetaInfo & mesh, std::vector<tuple3f> & target )
{
	target.clear();
	std::vector<tuple3i> & fcs =mesh.getBasicMesh().getFaces();
	std::vector<tuple3f> & verts =mesh.getBasicMesh().getVertices();
	target.reserve(fcs.size());
	
	std::vector<tuple3i>::iterator fc;
	/*tuple3f normal_ac;
	tuple3f normal_ab;
	tuple3f normal;*/
	tuple3f circumcenter;
	float l_ab_sqr, l_bc_sqr, l_ca_sqr, bari_a,bari_b, bari_c, tot;
	for(fc = fcs.begin(); fc != fcs.end(); fc++){
		/*normal = (verts[fc->c] -verts[fc->a]).cross(verts[fc->b]-verts[fc->a]);
		normal_ac = (verts[fc->c] -verts[fc->a]).cross(normal);
		normal_ac.normalize();*/
		l_ab_sqr = (verts[fc->b] -verts[fc->a]).normSqr();
		l_bc_sqr = (verts[fc->c] -verts[fc->b]).normSqr();
		l_ca_sqr = (verts[fc->a] -verts[fc->c]).normSqr();
		bari_a = l_bc_sqr*(-l_bc_sqr + l_ca_sqr + l_ab_sqr);
		bari_b = l_ca_sqr*(l_bc_sqr - l_ca_sqr + l_ab_sqr);
		bari_c = l_ab_sqr*(l_bc_sqr + l_ca_sqr - l_ab_sqr);

		tot = bari_a + bari_b + bari_c;
		circumcenter = verts[fc->a] * (bari_a /tot) + verts[fc->b] * (bari_b /tot)
			+ verts[fc->c] * (bari_c /tot);
		target.push_back(circumcenter);

		float tmp = (verts[fc->a]-verts[fc->b]).dot((verts[fc->a] + verts[fc->b])*0.5f -target.back());
		assert(tmp < 0.000001 && tmp > -0.000001);
		tmp = (verts[fc->c]-verts[fc->b]).dot((verts[fc->c] + verts[fc->b])*0.5f -target.back());
		assert(tmp < 0.000001 && tmp > -0.000001);
		tmp = (verts[fc->a]-verts[fc->c]).dot((verts[fc->a] + verts[fc->c])*0.5f -target.back());
		assert(tmp < 0.000001 && tmp > -0.000001);
	}
}
