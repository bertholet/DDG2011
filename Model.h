#pragma once
#include "mesh.h"
#include "meshMetaInfo.h"
#include "VectorField.h"
#include "constraintCollector.h"

/************************************************************************/
/* Singleton where the actual mesh is stored and the actual state of the Program is encoded*/
/************************************************************************/
class Model
{
protected:
	Model(void);

public:
	static Model* getModel();
	~Model(void);

	mesh* getMesh();
	meshMetaInfo * getMeshInfo();
	void setMesh(mesh * aMesh);

	VectorField * getVField();
	void setVField( VectorField * field );
	void initVectorField();

	fieldConstraintCollector & getInputCollector();
	
private:
	static Model* instance;
	mesh * myMesh;
	meshMetaInfo * metaInfo;
	VectorField * vField;

	fieldConstraintCollector collector;

	bool vFieldValid;
};
