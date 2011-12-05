#include "meshMetaInfo.h"
#include "Operator.h"
#include "meshOperation.h"

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
