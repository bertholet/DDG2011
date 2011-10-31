#include "meshMetaInfo.h"
#include "Operator.h"

meshMetaInfo::meshMetaInfo(mesh * aMesh)
{
	aMesh->attach(this);
	curvNormalsActive = false;
	curvNormalsValid = false;
	myMesh =aMesh;
}

meshMetaInfo::~meshMetaInfo(void)
{
}

void meshMetaInfo::update(void * src, int type )
{
	if(type == mesh::MESHVERTICESCHANGED && src == myMesh){
		curvNormalsValid = false;
	}
}

void meshMetaInfo::activateCurvNormals( bool activated )
{
	this->curvNormalsValid = false;
	this->curvNormalsActive = activated;
	Operator::calcAllCurvNormals(*myMesh, curvatureNormals);
	if(!activated){
		curvatureNormals.clear();
	}
}

bool meshMetaInfo::curvNormalsAcitvated()
{
	return curvNormalsActive;
}

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
