#include "Model.h"
#include <algorithm>
#include <assert.h>

Model * Model::instance = 0;

Model::Model(void)
{
	myMesh = new mesh();
	metaInfo = new meshMetaInfo(myMesh);
	vField = NULL;
	vFieldValid = true;
	points = NULL;
	positions = NULL;
	dirs = NULL;
	fluidSim = NULL;
	displayLength = 1.f;
	showArrows = false;
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
	fluidSim = NULL;

	points = NULL;
	dirs = NULL;
	positions = NULL;
	collector.clear();

	this->updateObserver(NEW_MESH_CREATED);
}

void Model::setVField( VectorField * field )
{
	if(this->vField != NULL){
		delete vField;
	}
	vField = field;
	vField->setShowArrows(showArrows);

	updateObserver(DISPLAY_CHANGED);
}

VectorField * Model::getVField()
{
	if(vField == NULL){
		initVectorField();
		this->vField->setShowArrows(showArrows);
	}
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

void Model::attach( Observer<modelMsg> * obs )
{
	this->observer.push_back(obs);
}

void Model::detatch( Observer<modelMsg> * obs )
{
	vector<Observer<modelMsg> * >::iterator it;
	it = find(observer.begin(), observer.end(), obs);
	if(it != observer.end()){
		this->observer.erase(it);
	}
	else{
		assert(false);
	}

}

void Model::updateObserver( modelMsg msg )
{
	for(int i = 0; i < observer.size(); i++){
		observer[i]->update(this, msg);
	}
}

std::vector<tuple3f>  * Model::getPointCloud()
{
	return this->points;
}

std::vector<std::vector<int>>  * Model::getPointCloudConnectivity()
{
	return this->dualF2V;
}

void Model::setDualMesh( std::vector<tuple3f> * points_ ,std::vector<std::vector<int>> * dualF2V_)
{
	this->points = points_;
	this->dualF2V = dualF2V_;
	this->updateObserver(DISPLAY_CHANGED);
}

void Model::setVectors( std::vector<tuple3f>* dualVertices, std::vector<tuple3f>* velocities, 
					   bool updateObservers)
{
	this->positions = dualVertices;
	this->dirs = velocities;
	if (updateObservers){
		this->updateObserver(DISPLAY_CHANGED);
	}
}

void Model::setDisplayLength( float param1 )
{
	displayLength = param1;
}

std::vector<tuple3f> * Model::getPos()
{
	return positions;
}

std::vector<tuple3f> * Model::getDirs()
{
	return dirs;
}

fluidSimulation * Model::getFluidSimulation()
{
	return fluidSim;
}

void Model::setFluidSim( fluidSimulation * sim )
{
	fluidSim = sim;
}

void Model::setShowArrows( bool show )
{
	showArrows = show;
	if(this->vField != NULL){
		this->vField->setShowArrows(show);
	}

	this->updateObserver(DISPLAY_CHANGED);

}

bool Model::getShowArrows( void )
{
	return showArrows;
}
