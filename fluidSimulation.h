#pragma once
#include <vector>
#include "tuple3.h"
#include "meshMetaInfo.h"
#include "oneForm.h"
#include "nullForm.h"
#include "pardisoMatrix.h"
#include "DDGMatrices.h"
#include "colorMap.h"

class fluidSimulation:public colorMap
{
private:
	meshMetaInfo * myMesh;
	std::vector<tuple3f> velocities;
	std::vector<tuple3f> dualVertices;
	
	std::vector<tuple3f> backtracedDualVertices;
	//the triangle the backtraced dual vertex lies in.
	std::vector<int> triangle_btVel;

	//for visualisation: line stripes start at some position and wander
	//around
	vector<tuple3f> line_stripe_starts;
	vector<int> line_strip_triangle;
	vector<int> age;
	int maxAge;

	std::vector<tuple3f> backtracedVelocity;
	oneForm flux;
	oneForm forceFlux;
	nullForm vorticity;
	nullForm tempNullForm;
	//L^-1 * Vorticity is stored here.
	nullForm L_m1Vorticity;

	//oneForm2oneForm Laplacian
	pardisoMatrix L;
	pardisoMatrix d0;
	pardisoMatrix dt_star1;
	pardisoMatrix star0_,star0_inv;
	pardisoMatrix star0_min_vhl;

	//the viscosity. surprise surprise...
	float viscosity;
	//timeStep
	float timeStep;

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

	void setForce(vector<tuple3f> & dirs);

	void setViscosity(float visc);
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
	void walkPath(tuple3f * pos, int * triangle, float *  t, int dir =-1);

	float maxt( tuple3f & pos, int triangle, tuple3f & dir, tuple3f & cutpos, tuple2i & edge );


	//////////////////////////////////////////////////////////////////////////
	// Method to iterpolate the velocity field on curved manifolds. 
	// The Velocities are projected locally along the
	// curvature normal onto a plane, interpolated there and then projected 
	// back
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
	void addForces2Vorticity(float timestep);
	void setStepSize( float stepSize );
	void addDiffusion2Vorticity();

/////////////////////////////////////////////////////////////////////////
// display the field
//////////////////////////////////////////////////////////////////////////
	void glDisplayField();
	float texPos( int j, int nrPoints );


//////////////////////////////////////////////////////////////////////////
//colormap Methods
//////////////////////////////////////////////////////////////////////////
	virtual tuple3f color( int vertexNr );

	virtual std::string additionalInfo( void );

};
