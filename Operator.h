#pragma once
#include "tuple3.h"
#include "mesh.h"
#include <vector>

class Operator
{

public:
	Operator(void);
	~Operator(void);
	static void calcAllCurvNormals(mesh &m, vector<tuple3f> &target);
	static void calcAllGaussCurvs(mesh &m, vector<float> &target);
	static void calcAllAMixed(mesh &m, vector<float> &target);
	static float volume(mesh &m);
	static float area(int faceNr, mesh & m);

	//Vornoi area around the vertex vertNr. Look out, does not give anything smart on borders
	//but does not fail either.
	static float aVornoi(int vertNr, mesh & m);

	//cotan phik + cotan phil.
	static float dualEdge_edge_ratio(int i, int j, mesh & m);

	//largest over smallest area ratio
	static float maxAreaRatio(mesh & m);
};
