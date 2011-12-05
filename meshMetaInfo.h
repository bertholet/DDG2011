#pragma once
#include "mesh.h"
#include "Observer.h"
#include <vector>
#include "tuple3.h"

/************************************************************************/
/* This class contains information like border, normal curvature etc
/* if computed.
/************************************************************************/
class meshMetaInfo:Observer<meshMsg>
{
private:
	mesh * myMesh;
	std::vector<tuple3f> curvatureNormals;
	bool curvNormalsActive, curvNormalsValid;
	std::vector<tuple3i> fc_halfedges;
	std::vector<tuple2i> halfedges;
	bool halfedges_active, halfedges_valid;
	vector<vector<int>> border;
	bool border_valid;

public:
	meshMetaInfo(mesh * aMesh);
	~meshMetaInfo(void);
	
	//overloaded from observer
	void update(void * src, meshMsg type);

	//if curvatures are deactivated the internal memory reserved for them is cleared.-
	//void activateCurvNormals(bool activated);
	
	//bool curvNormalsAcitvated();

	vector<tuple3f> * getCurvNormals();
	vector<tuple2i> * getHalfedges();
	vector<tuple3i> * getFace2Halfedges();
	vector<vector<int>> & getBorder();
};
