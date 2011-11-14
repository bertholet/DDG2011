#include "Model.h"

Model * Model::instance = 0;

Model::Model(void)
{
	myMesh = new mesh();
	metaInfo = new meshMetaInfo(myMesh);
	vField = NULL;
	vFieldValid = true;
}

Model::~Model(void)
{
	if(myMesh!= NULL){
		delete myMesh;
	}
	if(metaInfo!= NULL){
		delete metaInfo;
	}
	if(vField != NULL){
		delete vField;
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
	if(vField != NULL){
		delete vField;
	}
	myMesh = aMesh;
	metaInfo = new meshMetaInfo(myMesh);
	vField = NULL;
}

void Model::setVField( VectorField * field )
{
	if(this->vField != NULL){
		delete vField;
	}
	vField = field;
}

VectorField * Model::getVField()
{
	return this->vField;
}

void Model::initVectorField()
{
	vField = new VectorField(myMesh);
}

fieldConstraintCollector & Model::getInputCollector()
{
	return this->collector;
}
