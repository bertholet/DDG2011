#pragma once
#include <vector>
#include "tuple3.h"
#include "meshMetaInfo.h"
#include "oneForm.h"
#include "twoForm.h"

class fluidSimulation
{
private:
	meshMetaInfo * myMesh;
	std::vector<tuple3f> velocities;
	std::vector<tuple3f> dualVertices;
	std::vector<tuple3f> backtracedDualVertices;
	oneForm flux;
	twoForm vorticity;

public:

	//////////////////////////////////////////////////////////////////////////
	// take(n) care of outside to destroy the fluid simulation if mesh is changed
	// or destroyed.
	//////////////////////////////////////////////////////////////////////////
	fluidSimulation(meshMetaInfo * mesh);
	~fluidSimulation(void);

	//////////////////////////////////////////////////////////////////////////
	// setter. Note if mesh does not coincide an assertion fails
	//////////////////////////////////////////////////////////////////////////
	void setFlux( oneForm & f );

	//////////////////////////////////////////////////////////////////////////
	// Pathtrace all dualvertices
	//
	//////////////////////////////////////////////////////////////////////////
	void pathTraceDualVertices(float t);

	//////////////////////////////////////////////////////////////////////////
	// walk the path for at most a timestep t in the given triangle. t is updated
	// to the remaining t to go, pos is updated and the new actual triangle
	// if the border of the triangle is reached.
	// If the border of the mesh is reached -1 is returned.
	//////////////////////////////////////////////////////////////////////////
	void walkPath(tuple3f * pos, int * triangle, float *  t);

	float maxt( tuple3f & pos, int triangle, tuple3f & dir, tuple3f & cutpos, tuple2i & edge );


	//////////////////////////////////////////////////////////////////////////
	// Debug-Display Dual vertices
	//////////////////////////////////////////////////////////////////////////
	void showDualPositions();

	//////////////////////////////////////////////////////////////////////////
	// pathtrace and display pathtracedPositions
	// Again this is for debug / visualisation reasons
	//////////////////////////////////////////////////////////////////////////
	void pathTraceAndShow(float howmuch);
	void showFlux2Vel();


	//////////////////////////////////////////////////////////////////////////
	// Method for curved manifolds. The Velocities are rectified along the
	// curvature normal and then projected back
	//////////////////////////////////////////////////////////////////////////
	tuple3f getVelocityFlattened(tuple3f & pos, int triangleIndex, tuple3i & tr );


	//////////////////////////////////////////////////////////////////////////
	// helpmethod that interpolates the velocityfield defined on the dualvertex
	// positions. triangleIndex is the index of the triangle containing pos.
	//////////////////////////////////////////////////////////////////////////
	tuple3f getVelocity( tuple3f & pos, int triangleIndex, tuple3i & tr );
	//////////////////////////////////////////////////////////////////////////
	//project velocity on the triangle and scale it such that the length
	// stays. This is used for interpolation of velocities ofver triangle
	// borders, between neiighboring triangles.
	//////////////////////////////////////////////////////////////////////////
	tuple3f project( tuple3f& velocity, int actualTriangle );
};
