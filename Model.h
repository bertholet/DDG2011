#pragma once
#include "mesh.h"
#include "meshMetaInfo.h"
#include "VectorField.h"
#include "constraintCollector.h"
#include "Observer.h"
#include <vector>
#include "fluidSimulation.h"

/************************************************************************/
/* Singleton where the actual mesh is stored and the actual state of the Program is encoded*/
/************************************************************************/
class Model
{
protected:
	Model(void);

public:
	enum modelMsg{NEW_MESH_CREATED,DISPLAY_CHANGED};

	static Model* getModel();
	~Model(void);

	mesh* getMesh();
	meshMetaInfo * getMeshInfo();

	//////////////////////////////////////////////////////////////////////////
	// The Memory Management of mesh is taken care of by model.
	// i.e. it is deleted at destruction or mesh change.
	//////////////////////////////////////////////////////////////////////////
	void setMesh(mesh * aMesh);

	VectorField * getVField();

	//////////////////////////////////////////////////////////////////////////
	// The Memory Management of field is taken care of by model.
	// i.e. it is deleted at destruction or mesh change.
	//////////////////////////////////////////////////////////////////////////
	void setVField( VectorField * field );
	void initVectorField();

	//////////////////////////////////////////////////////////////////////////
	// Will make the displayer display these points.
	// Note: the Memory of points needs to be released elsewhere
	// at the time given-..
	//////////////////////////////////////////////////////////////////////////
	void setPointCloud(std::vector<tuple3f> * points);
	std::vector<tuple3f>  * getPointCloud();

	//////////////////////////////////////////////////////////////////////////
	// Will make the displayer display these Vectors.
	// Note: the Memory of points needs to be released elsewhere
	// at the time given-..
	//////////////////////////////////////////////////////////////////////////
	void setVectors( std::vector<tuple3f>* pos, std::vector<tuple3f>* dir );
	std::vector<tuple3f> * getPos();
	std::vector<tuple3f> * getDirs();
	void setDisplayLength( float param1 );
	float getDisplayLength(){return displayLength;}

	//////////////////////////////////////////////////////////////////////////
	// Will let the fluid simulation display. The memory management is not
	// taken care of, i.e. the fluid sim has to be deleted elsewhere
	//////////////////////////////////////////////////////////////////////////
	void setFluidSim(fluidSimulation * sim);
	fluidSimulation * getFluidSimulation();

	fieldConstraintCollector & getInputCollector();

	void attach(Observer<modelMsg> * obs);
	void detatch(Observer<modelMsg> * obs);
	void updateObserver(modelMsg msg);

private:
	static Model* instance;
	mesh * myMesh;
	meshMetaInfo * metaInfo;
	VectorField * vField;

	//a list of vectors to display
	std::vector<tuple3f> * positions;
	std::vector<tuple3f> * dirs;

	// just any list of points you want to be displayed.
	// good for debugging
	std::vector<tuple3f> * points;

	vector<Observer<modelMsg> *> observer;

	fieldConstraintCollector collector;

	bool vFieldValid;
	float displayLength;
	fluidSimulation * fluidSim;
};
