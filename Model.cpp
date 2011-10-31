#include "Model.h"

Model * Model::instance = 0;

Model::Model(void)
{
	myMesh = NULL;
}

Model::~Model(void)
{
	if(myMesh!= NULL){
		delete myMesh;
	}
}

Model* Model::getModel()
{
	if(Model::instance == 0){
		Model::instance = new Model();
	}
	return Model::instance;
}

mesh* Model::getMesh()
{
	return myMesh;
}

void Model::setMesh( mesh * aMesh )
{
	myMesh = aMesh;
}
