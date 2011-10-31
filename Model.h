#pragma once
#include "mesh.h"
#include "meshMetaInfo.h"

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
private:
	static Model* instance;
	mesh * myMesh;
	meshMetaInfo * metaInfo;
};
