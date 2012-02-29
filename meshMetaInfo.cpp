#include "meshMetaInfo.h"
#include "Operator.h"
#include "meshOperation.h"
#include <algorithm>

meshMetaInfo::meshMetaInfo(mesh * aMesh)
{
	aMesh->attach(this);
	curvNormalsActive = false;
	curvNormalsValid = false;
	halfedges_active = false;
	halfedges_valid = false;
	border_valid = false;
	myMesh =aMesh;
}

meshMetaInfo::~meshMetaInfo(void)
{
}

void meshMetaInfo::update(void * src, meshMsg type )
{
	if(type == POS_CHANGED && src == myMesh){
		curvNormalsValid = false;
	}
	else if(type == CONNECTIVITY_CHANGED && src == myMesh){
		curvNormalsValid = false;
		halfedges_valid = false;
		border.clear();
		border_valid = false;
	}
}

/*void meshMetaInfo::activateCurvNormals( bool activated )
{
	this->curvNormalsValid = false;
	this->curvNormalsActive = activated;
	Operator::calcAllCurvNormals(*myMesh, curvatureNormals);
	if(!activated){
		curvatureNormals.clear();
	}
}*/

/*bool meshMetaInfo::curvNormalsAcitvated()
{
	return curvNormalsActive;
}*/

// With lazy initialization.
vector<tuple3f> * meshMetaInfo::getCurvNormals()
{
	if(curvNormalsValid){
		return &curvatureNormals;
	}
	else{
		Operator::calcAllCurvNormals(*myMesh, curvatureNormals);
		curvNormalsValid = true;
	}
	curvNormalsActive = true;

	return &curvatureNormals;
}

vector<tuple2i> * meshMetaInfo::getHalfedges()
{
	if(halfedges_valid){
		return & halfedges;
	}
	else{
		meshOperation::getOrientedEdges(*myMesh,fc_halfedges,halfedges);
		halfedges_valid = true;
	}
	halfedges_active = true;
	return & halfedges;
}

//////////////////////////////////////////////////////////////////////////
// Note the half edges are ordered by occurrence: return[i].a is the (pos OR
// negatively oriented seen from face i, both is possible ) halfedge 
// between face[i].a and face[i].b and so on.
//////////////////////////////////////////////////////////////////////////
vector<tuple3i> * meshMetaInfo::getFace2Halfedges()
{
	if(halfedges_valid){
		return & fc_halfedges;
	}
	else{
		meshOperation::getOrientedEdges(*myMesh,fc_halfedges,halfedges);
		halfedges_valid = true;
	}
	halfedges_active = true;
	return & fc_halfedges;
}

vector<vector<int>> & meshMetaInfo::getBorder()
{
	if(!border_valid){
		meshOperation::getBorder(*myMesh,border);
		border_valid = true;
	}
	return border;
}

int meshMetaInfo::getHalfedgeId( int a, int b, tuple2i * target )
{
	vector<tuple2i> & he = * getHalfedges();
	tuple2i toFind;
	if(a < b){
		toFind.set(a,b);
	}
	else if (b < a){
		toFind.set(b,a);
	}
	else{assert(false);}

	vector<tuple2i>::iterator found;
	found =lower_bound(he.begin(), he.end(), toFind);
	int index=-1;
	if(found->a == toFind.a && found->b == toFind.b){
		index = found - he.begin();
	}
	if(target != NULL){
		if(index >-1){
			target->set(toFind.a, toFind.b);
		}
		else{
			target->set(-1,-1);
		}
	}

	return index;



}
