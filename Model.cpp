#include "Model.h"

Model * Model::instance = 0;

Model::Model(void)
{
	myMesh = new mesh();
	metaInfo = new meshMetaInfo(myMesh);
}

Model::~Model(void)
{
	if(myMesh!= NULL){
		delete myMesh;
	}
	if(metaInfo!= NULL){
		delete metaInfo;
	}
}

Model* Model::getModel()
{
	if(Model::instance == NULL){
		Model::instance = new Model();
	}
	return Model::instance;
}

//////////////////////////////////////////////////////////////////////////
//getters and setters
//////////////////////////////////////////////////////////////////////////

mesh* Model::getMesh()
{
	return myMesh;
}

meshMetaInfo* Model::getMeshInfo()
{
	return metaInfo;
}

void Model::setMesh( mesh * aMesh )
{
	if(myMesh != NULL){
		delete myMesh;
	}
	if(metaInfo != NULL){
		delete metaInfo;
	}
	myMesh = aMesh;
	metaInfo = new meshMetaInfo(myMesh);

}
