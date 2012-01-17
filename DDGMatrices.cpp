#include "DDGMatrices.h"
#include <vector>
#include "pardisoMatCreator.h"
#include "tuple3.h"
#include "Operator.h"

class d_0Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;

public:
	d_0Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
	}

	float val(int i , int j){
		// i is the row
		tuple2i & edge =(*mesh->getHalfedges())[i];
		return edge.orientation(j);
	}

	// row: its the edge number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		tuple2i & edge =(*mesh->getHalfedges())[row];
		target.push_back(edge.a);
		target.push_back(edge.b);
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
		tuple3i & f2e =(*mesh->getFace2Halfedges())[i];
		
		return f2e.orientation((*mesh->getHalfedges())[j]);
	}

	// row: its the face number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		tuple3i & f2e =(*mesh->getFace2Halfedges())[row];
		target.push_back(f2e.a);
		target.push_back(f2e.b);
		target.push_back(f2e.c);
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
		return Operator.aVornoi(i, mesh->getBasicMesh());
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
		// i is the row
		return Operator.aVornoi(i, mesh->getBasicMesh());
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

//0 for surface meshes, as this is all for now. 
pardisoMatrix DDGMatrices::d2( meshMetaInfo & aMesh )
{
	pardisoMatrix mat;
	return mat;
}


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
	star0.initMatrix(star1Creator(aMesh), nrEdges);
	return star0;
}


