#pragma once
#include "mesh.h"
#include "Observer.h"
#include <vector>
#include "tuple3.h"

/************************************************************************/
/* This class contains information like border, normal curvature etc
/* if computed.
/************************************************************************/
class meshMetaInfo:Observer
{
private:
	std::vector<tuple3f> curvatureNormals;
	bool curvNormalsActive, curvNormalsValid;
	mesh * myMesh;

public:
	meshMetaInfo(mesh * aMesh);
	~meshMetaInfo(void);
	
	//overloaded from observer
	void update(void * src, int type);

	//if curvatures are deactivated the internal memory reserved for them is cleared.-
	void activateCurvNormals(bool activated);
	bool curvNormalsAcitvated();

	vector<tuple3f> * getCurvNormals();

};
