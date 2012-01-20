#pragma once
#include "mesh.h"
#include "meshMetaInfo.h"
#include "VectorField.h"
#include "constraintCollector.h"
#include "Observer.h"
#include <vector>

/************************************************************************/
/* Singleton where the actual mesh is stored and the actual state of the Program is encoded*/
/************************************************************************/
class Model
{
protected:
	Model(void);

public:
	enum modelMsg{NEW_MESH_CREATED};

	static Model* getModel();
	~Model(void);

	mesh* getMesh();
	meshMetaInfo * getMeshInfo();
	void setMesh(mesh * aMesh);

	VectorField * getVField();
	void setVField( VectorField * field );
	void initVectorField();

	std::vector<tuple3f>  * getPointCloud();
	void setPointCloud(std::vector<tuple3f> * points);

	fieldConstraintCollector & getInputCollector();

	void attach(Observer<modelMsg> * obs);
	void detatch(Observer<modelMsg> * obs);
	void updateObserver(modelMsg msg);

private:
	static Model* instance;
	mesh * myMesh;
	meshMetaInfo * metaInfo;
	VectorField * vField;
	// just any list of points you want to be displayed.
	// good for debugging
	std::vector<tuple3f> * points;

	vector<Observer<modelMsg> *> observer;

	fieldConstraintCollector collector;

	bool vFieldValid;
};
