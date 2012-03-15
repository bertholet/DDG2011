#include "DDGMatrices.h"
#include <vector>
#include "pardisoMatCreator.h"
#include "tuple3.h"
#include "Operator.h"
#include <algorithm>
#include <limits>
#include "idCreator.h"
#include "meshOperation.h"

class d_0Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	d_0Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		// i is the row, j the vertex
		tuple2i & edge =(*mesh->getHalfedges())[i];
		return edge.orientation(j);
	}

	// row: its the edge number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		tuple2i & edge =(*mesh->getHalfedges())[row];
		if( edge.a < edge.b){
			target.push_back(edge.a);
			target.push_back(edge.b);
		}
		else{
			target.push_back(edge.b);
			target.push_back(edge.a);
		}
	}
};

class d_1Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	d_1Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		// i is the row
		tuple3i & fc =(mesh->getBasicMesh().getFaces())[i];
		
		return fc.orientation((*mesh->getHalfedges())[j]);
	}

	// row: its the face number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		tuple3i & f2e =(*mesh->getFace2Halfedges())[row];
		target.push_back(f2e.a);
		target.push_back(f2e.b);
		target.push_back(f2e.c);

		sort(target.begin(), target.end());
	}
};


class star0Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	star0Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		if(i!= j){
			return 0;
		}


		// i is the row
		float temp =Operator::aVornoi(i, mesh->getBasicMesh());
		assert(temp < numeric_limits<float>::infinity());
		assert(temp > -numeric_limits<float>::infinity());

		return temp;
	}

	// row: its the vertex number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		target.push_back(row);
	}
};


class star1Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	star1Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		if(i!= j){
			return 0;
		}

		tuple2i & edge = (*mesh->getHalfedges())[i];
		// i is the row
		return Operator::dualEdge_edge_ratio(edge.a, edge.b,mesh->getBasicMesh());
	}

	// row: its the vertex number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		target.push_back(row);
	}
};

class star2Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	star2Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		if(i!= j){
			return 0;
		}

		// i is the row
		return 1.0 / Operator::area(i,mesh->getBasicMesh());
	}

	// row: its the vertex number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		target.push_back(row);
	}
};


class dualVal1Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	dualVal1Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	// i is the edge for which the dual value shall be calculated.§
	float val(int i , int j){
		if(i== j){
			return 0;
		}

		// i is the row
		tuple2i & edg = (* mesh->getHalfedges())[i];
		int nbr_fc1, nbr_fc2;
		meshOperation::getNbrFaces(edg,&nbr_fc1,&nbr_fc2, mesh->getBasicMesh().getNeighborFaces());
		
		//if i not on the border.
		if(nbr_fc2 >=0 ){
			return 0;
		}


		tuple3i he = (* mesh->getFace2Halfedges())[nbr_fc1];
		tuple3i fc = mesh->getBasicMesh().getFaces()[nbr_fc1];		
/*		if((!he.contains(j))&& nbr_fc2>=0){
			he = (* mesh->getFace2Halfedges())[nbr_fc2];
			fc = mesh->getBasicMesh().getFaces()[nbr_fc2];		
		}*/


		std::vector<tuple3f> & verts = mesh->getBasicMesh().getVertices();

		float a_ij = 0;
		int sign=1;
		if(j== he.a){
			a_ij = tuple3f::cotPoints(verts[fc.b],verts[fc.c],verts[fc.a]);
			sign = (i == he.b? -1: 1);
			sign *= fc.orientation((* mesh->getHalfedges())[he.a]);
			sign *= fc.orientation(edg);
			assert(sign!=0);
			a_ij*=sign;
		}
		else if(j== he.b){
			a_ij = tuple3f::cotPoints(verts[fc.c],verts[fc.a],verts[fc.b]);
			sign = (i == he.c? -1: 1);
			sign *= fc.orientation((* mesh->getHalfedges())[he.b]);
			sign *= fc.orientation(edg);
			assert(sign!=0);
			a_ij*=sign;
		}
		else if(j==he.c){
			a_ij = tuple3f::cotPoints(verts[fc.a],verts[fc.b],verts[fc.c]);
			sign = (i == he.a? -1: 1);
			sign *= fc.orientation((* mesh->getHalfedges())[he.c]); //reason for this: formula for e_ab, e_bc, e_ca i.e. all edges should be 
			//positive oriented
			sign *= fc.orientation(edg); //same.
			assert(sign!=0);
			a_ij*=sign;
		}
		else{
			assert(false);
		}

		/*if(nbr_fc2>=0){
			a_ij*=0.5;
		}*/
		return a_ij*-1;
	}

	// row: is the edge number; target will be the two other edges in the triangle; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		tuple2i & edg = (* mesh->getHalfedges())[row];
		int nbr_fc1, nbr_fc2;
		meshOperation::getNbrFaces(edg,&nbr_fc1,&nbr_fc2, mesh->getBasicMesh().getNeighborFaces());

		//if is border edge (i.e. we want to calculate dual value for this edge!)
		if(nbr_fc2 < 0){
			tuple3i he = (* mesh->getFace2Halfedges())[nbr_fc1];
			target.push_back(he.a);
			target.push_back(he.b);
			target.push_back(he.c);

	/*		if(nbr_fc2 >= 0){
				tuple3i he2 = (* mesh->getFace2Halfedges())[nbr_fc2];
				if(!he.contains(he2.a))
					target.push_back(he2.a);
				if(!he.contains(he2.b))
					target.push_back(he2.b);
				if(!he.contains(he2.c))
					target.push_back(he2.c);
			}*/

			sort(target.begin(),target.end());
		}
		
	}
};

//////////////////////////////////////////////////////////////////////////
// primal part of dual d1 with bordered mehs-
// d1_dual_primal times the dual values  on the primal edges
// gives the part of d1...
class d1dual_primal: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	d1dual_primal(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}
	
	float val(int i , int j){
	//j is an edge, i is a vertex.
		tuple2i & edge = (* mesh->getHalfedges())[j];
		if(edge.a != i && edge.b != i ){
			return 0;
		}

		int nbr_fc1, nbr_fc2;
		meshOperation::getNbrFaces(edge,&nbr_fc1,&nbr_fc2, mesh->getBasicMesh().getNeighborFaces());
		
		if(nbr_fc2 >=0){
			assert(nbr_fc1>=0);
			return 0;
		}
		//j is a border vertex adjascent to vertex i.
		//border vertices
		if(nbr_fc2<0){
			tuple3i fc = mesh->getBasicMesh().getFaces()[nbr_fc1];	
			assert(fc.contains(i));
			int or = fc.orientation(edge);
			assert(or!=0);
			// plus 0.5 in fc pos oriented edge value of incoming and of outgoing border edge.
			return 0.5 * or;
		}

		assert(false);
		return 0;
	}

	void indices(int row, std::vector<int> & target){
	//row is a vertex i.e dual face

		target.clear();
		if(meshOperation::isOnBorder(row,mesh->getBasicMesh())){
			meshOperation::getNeighborEdges(row,mesh->getBasicMesh().getNeighborFaces(),* mesh->getFace2Halfedges(),
			* mesh->getHalfedges(),target);
			sort(target.begin(),target.end());
		}
	}
};

class onesBorderEdgesCreator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	onesBorderEdgesCreator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		if(i!= j){
			return 0;
		}

		tuple2i & edge = (* mesh->getHalfedges())[i];

		int nbr_fc1, nbr_fc2;
		meshOperation::getNbrFaces(edge,&nbr_fc1,&nbr_fc2, mesh->getBasicMesh().getNeighborFaces());

		if(nbr_fc2 >=0){
			return 0;
		}
		return 1;
	}

	// row: its the vertex number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		target.push_back(row);
	}
};

DDGMatrices::DDGMatrices(void)
{
}

DDGMatrices::~DDGMatrices(void)
{
}


pardisoMatrix DDGMatrices::d0( meshMetaInfo & aMesh )
{
	pardisoMatrix d_0;
	int nrEdges = aMesh.getHalfedges()->size();
	d_0.initMatrix(d_0Creator(aMesh), nrEdges);

	return d_0;
}


pardisoMatrix DDGMatrices::d1( meshMetaInfo & aMesh )
{
	pardisoMatrix d_1;
	int nrFaces = aMesh.getBasicMesh().getFaces().size();
	d_1.initMatrix(d_1Creator(aMesh), nrFaces);
	return d_1;
}

pardisoMatrix DDGMatrices::dual_d0( meshMetaInfo & aMesh )
{
	pardisoMatrix d1_=d1(aMesh);
	return (id1(aMesh) % d1_);
}

pardisoMatrix DDGMatrices::dual_d1( meshMetaInfo & aMesh )
{
	pardisoMatrix d_0= d0(aMesh);
	d_0*=(-1);//^1
	return (id0(aMesh) % d_0);
}


pardisoMatrix DDGMatrices::dual_d1_borderdiff( meshMetaInfo & aMesh )
{
	pardisoMatrix dual1 = dualVals1(aMesh);
	pardisoMatrix d1diff;
	//dual edges to dual faces i.e. vertices
	d1diff.initMatrix(d1dual_primal(aMesh),aMesh.getBasicMesh().getVertices().size());
	d1diff.forceNrColumns(aMesh.getHalfedges()->size());

	pardisoMatrix star1inv = star1(aMesh);
	star1inv.elementWiseInv(0.000);
	return d1diff*dual1*star1inv;
}


pardisoMatrix DDGMatrices::dual_d1star1_borderdiff( meshMetaInfo & aMesh )
{
	pardisoMatrix dual1 = dualVals1(aMesh);
	pardisoMatrix d1diff;
	//dual edges to dual faces i.e. vertices
	d1diff.initMatrix(d1dual_primal(aMesh),aMesh.getBasicMesh().getVertices().size());
	d1diff.forceNrColumns(aMesh.getHalfedges()->size());
	return d1diff*dual1;
}


//0 for surface meshes, as this is all for now. 
/*pardisoMatrix DDGMatrices::d2( meshMetaInfo & aMesh )
{
	pardisoMatrix mat;
	return mat;
}*/


pardisoMatrix DDGMatrices::star0( meshMetaInfo & aMesh )
{
	pardisoMatrix star0;
	int nrVerts = aMesh.getBasicMesh().getVertices().size();
	star0.initMatrix(star0Creator(aMesh), nrVerts);
	return star0;
}

pardisoMatrix DDGMatrices::star1( meshMetaInfo & aMesh )
{
	pardisoMatrix star1;
	int nrEdges = aMesh.getHalfedges()->size();
	star1.initMatrix(star1Creator(aMesh), nrEdges);
	return star1;
}

pardisoMatrix DDGMatrices::star2( meshMetaInfo & aMesh )
{
	pardisoMatrix star2;
	int nrFaces = aMesh.getBasicMesh().getFaces().size();
	star2.initMatrix(star2Creator(aMesh), nrFaces);
	return star2;
}

pardisoMatrix DDGMatrices::delta1( meshMetaInfo & aMesh )
{
	pardisoMatrix star_1= star1(aMesh);
	//pardisoMatrix d_0 = d0(aMesh);
	pardisoMatrix duald1 = dual_d1(aMesh);

	pardisoMatrix star_0_inv = star0(aMesh);
	star_0_inv.elementWiseInv(0.0000000);

	return star_0_inv * duald1 * star_1;
	//return (star_0_inv % d_0)* star_1;


}

pardisoMatrix DDGMatrices::delta2( meshMetaInfo & aMesh )
{
	pardisoMatrix star_2= star2(aMesh);
	//pardisoMatrix d_1 = d1(aMesh);
	pardisoMatrix duald0 = dual_d0(aMesh);

	pardisoMatrix star_1_inv = star1(aMesh);
	star_1_inv.elementWiseInv(0.0000000);

	return star_1_inv * duald0 * star_2;
	//return (star_1_inv % d_1)* star_2;
}


pardisoMatrix DDGMatrices::id0( meshMetaInfo & aMesh )
{
	pardisoMatrix id;
	id.initMatrix(idCreator(), aMesh.getBasicMesh().getVertices().size());
	return id;
}

pardisoMatrix DDGMatrices::id1( meshMetaInfo & aMesh )
{
	pardisoMatrix id;
	id.initMatrix(idCreator(), aMesh.getHalfedges()->size());
	return id;
}

pardisoMatrix DDGMatrices::id2( meshMetaInfo & aMesh )
{
	pardisoMatrix id;
	id.initMatrix(idCreator(), aMesh.getBasicMesh().getFaces().size());
	return id;
}


pardisoMatrix DDGMatrices::dualVals1( meshMetaInfo & aMesh )
{
	pardisoMatrix dual1;
	dual1.initMatrix(dualVal1Creator(aMesh),aMesh.getHalfedges()->size());
	dual1.forceNrColumns(dual1.getn());
	assert(dual1.getn() == aMesh.getHalfedges()->size() &&
		dual1.getm() == aMesh.getHalfedges()->size() );
	return dual1;
}

//ones on border for border Vertices!
pardisoMatrix DDGMatrices::onesBorder( std::vector<std::vector<int>> & border, int n, int m /*= n*/ )
{
	pardisoMatrix ones;
	ones.initMatrix(sparseDiagCreator(&border),n);
	ones.forceNrColumns(m);
	return ones;
}

//edge x edge matrix with ones exactly for border edges.
pardisoMatrix DDGMatrices::onesBorderEdges( meshMetaInfo & aMesh  )
{
	pardisoMatrix ones;
	ones.initMatrix(onesBorderEdgesCreator(aMesh),aMesh.getHalfedges()->size());
	ones.forceNrColumns(aMesh.getHalfedges()->size());
	return ones;
}

pardisoMatrix DDGMatrices::dual_d1_dualprimal( meshMetaInfo & aMesh )
{
	pardisoMatrix dualprimal;
	//dual edges to dual faces i.e. vertices
	dualprimal.initMatrix(d1dual_primal(aMesh),aMesh.getBasicMesh().getVertices().size());
	dualprimal.forceNrColumns(aMesh.getHalfedges()->size());
	return dualprimal;
}




