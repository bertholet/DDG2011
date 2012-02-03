#pragma once
#include <vector>
#include "tuple3.h"
#include "meshMetaInfo.h"
#include "oneForm.h"
#include "nullForm.h"
#include "pardisoMatrix.h"
#include "DDGMatrices.h"

class fluidSimulation
{
private:
	meshMetaInfo * myMesh;
	std::vector<tuple3f> velocities;
	std::vector<tuple3f> dualVertices;
	
	std::vector<tuple3f> backtracedDualVertices;
	//the triangle the backtraced dual vertex lies in.
	std::vector<int> triangle_btVel;

	std::vector<tuple3f> backtracedVelocity;
	oneForm flux;
	nullForm vorticity;
	nullForm star0_inv_vort;
	//L^-1 * Vorticity is stored here.
	nullForm L_m1Vorticity;

	//oneForm2oneForm Laplacian
	pardisoMatrix L;
	pardisoMatrix d0;
	pardisoMatrix dt_star1;
	pardisoMatrix star0_inv;

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
	void setFlux( vector<tuple3f> & dirs );
	//////////////////////////////////////////////////////////////////////////
	// Pathtrace all dualvertices
	//
	//////////////////////////////////////////////////////////////////////////
	void pathTraceDualVertices(float t);


	//////////////////////////////////////////////////////////////////////////
	//
	// to calculate backtraced vorticity
	//////////////////////////////////////////////////////////////////////////
	void updateBacktracedVelocities();
	void backtracedVorticity();

	//////////////////////////////////////////////////////////////////////////
	// Vorticity 2 Flux
	//////////////////////////////////////////////////////////////////////////
	void vorticity2Flux();

	void flux2Vorticity();

	//////////////////////////////////////////////////////////////////////////
	// velocity2Flux
	// method to inject motion i.e. stirring etc
	//////////////////////////////////////////////////////////////////////////
	//void velocity2Flux();

	//////////////////////////////////////////////////////////////////////////
	// walk the path for at most a timestep t in the given triangle. t is updated
	// to the remaining t to go, pos is updated and the new actual triangle
	// if the border of the triangle is reached.
	// If the border of the mesh is reached -1 is returned.
	//////////////////////////////////////////////////////////////////////////
	void walkPath(tuple3f * pos, int * triangle, float *  t);

	float maxt( tuple3f & pos, int triangle, tuple3f & dir, tuple3f & cutpos, tuple2i & edge );


	//////////////////////////////////////////////////////////////////////////
	// Method for curved manifolds. The Velocities are rectified along the
	// curvature normal and then projected back
	//////////////////////////////////////////////////////////////////////////
	tuple3f getVelocityFlattened(tuple3f & pos, int triangleIndex );


//////////////////////////////////////////////////////////////////////////
	// ID IE IB IU IG   IS IT IU IF IF 
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

	void oneStep(float howmuuch);

	//////////////////////////////////////////////////////////////////////////
	// helpmethod that interpolates the velocityfield defined on the dualvertex
	// positions. triangleIndex is the index of the triangle containing pos.
	//////////////////////////////////////////////////////////////////////////
//	tuple3f getVelocity( tuple3f & pos, int triangleIndex, tuple3i & tr );

	//////////////////////////////////////////////////////////////////////////
	//project velocity on the triangle and scale it such that the length
	// stays. This is used for interpolation of velocities ofver triangle
	// borders, between neiighboring triangles.
	//////////////////////////////////////////////////////////////////////////
	tuple3f project( tuple3f& velocity, int actualTriangle );
	void updateVelocities();
	oneForm & getFlux();
};
