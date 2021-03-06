#pragma once
#include <vector>
#include "tuple3.h"
#include "meshMetaInfo.h"
#include "oneForm.h"
#include "nullForm.h"
#include "pardisoMatrix.h"
#include "DDGMatrices.h"
#include "colorMap.h"
#include <QTime>
#include "pardiso.h"


class fluidSimulation:public colorMap
{
private:
	meshMetaInfo * myMesh;

	// the actual simulation time.
	float simulationtime;

	// Things needed for Pathtracing
	std::vector<tuple3f> velocities;
	std::vector<tuple3f> velocities_vorticitypart;
	std::vector<tuple3f> dualVertices;
	std::vector<std::vector<int>> dualF2V;
	std::vector<tuple3f> backtracedDualVertices;

	//the triangle the backtraced dual vertex lies in.
	std::vector<int> triangle_btVel;
	//for fast borderdecision during pathtracing
	std::vector<bool> vertexOnBorder;

	//the harmonic field as velocities (in case of bordered meshs and constraints.
	std::vector<tuple3f> harmonicVelocities;

	//for visualisation: line stripes start at some position and wander
	//around
	vector<tuple3f> line_stripe_starts;
	vector<int> line_strip_triangle;
	vector<int> age;
	int maxAge;
	bool showStreamLines, doInterpolation;
	QTime lastFrame;
	QTime timer, timer_in_between, timer_total;

	//Forms
	std::vector<tuple3f> backtracedVelocity, backtracedVelocity_noHarmonic;
	oneForm flux;
	oneForm forceFlux;
	oneForm harmonicFlux;
	nullForm vorticity;
	nullForm tempNullForm;
	//L^-1 * Vorticity is stored here.
	nullForm L_m1Vorticity;

	//The Matrices needed
	pardisoMatrix L;
	pardisoMatrix d0;
	pardisoMatrix dt_star1;
	pardisoMatrix star0;

	pardisoMatrix star0_min_vhl;

	//the solvers;
	pardisoSolver * addDiffusionSolver;
	pardisoSolver * vort2FluxSolver;

	//the viscosity. surprise surprise...
	float viscosity;
	//timeStep
	float timeStep;
	float fps;


	double minVort, maxVort;
	int streamlineLength;
	bool showVortNotSpeed;
	bool boolTexLines;
	float colorScale;
public:

	//////////////////////////////////////////////////////////////////////////
	// take(n) care of outside to destroy the fluid simulation if mesh is changed
	// or destroyed.
	//////////////////////////////////////////////////////////////////////////
	fluidSimulation(meshMetaInfo * mesh);

	void initVertexOnBorder();

	void setupMatrices();

	~fluidSimulation(void);

	//////////////////////////////////////////////////////////////////////////
	// setter. Note if mesh does not coincide with the oneform's an assertion fails
	//////////////////////////////////////////////////////////////////////////
	void setFlux( oneForm & f );
	void setFlux( vector<tuple3f> & dirs );

	void setForce(vector<tuple3f> & dirs);

	void setViscosity(float visc);

	void setStepSize( float stepSize );
	

	//////////////////////////////////////////////////////////////////////////
	// the actual Fluidsimulation algorithm
	//////////////////////////////////////////////////////////////////////////
	void oneStep();

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
	void vorticity2Flux(nullForm & vort, oneForm & target);
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
	// weight_buffer is a vector that is used for intern calculations as
	// preallocated memory.
	//////////////////////////////////////////////////////////////////////////
	void walkPath(tuple3f * pos, int * triangle, float *  t, bool * hitBorder,
		std::vector<float> & weight_buffer,int dir =-1);

	float maxt( tuple3f & pos, int triangle, tuple3f & dir, tuple3f & cutpos, tuple2i & edge );


	//////////////////////////////////////////////////////////////////////////
	// Method to iterpolate the velocity field on curved manifolds. 
	// The Velocities are projected locally along the
	// curvature normal onto a plane, interpolated there and then projected 
	// back. The resulting velocity will be stored in result, weightbuffer
	// needs to be a vector, used as a buffer for intern calculations.
	//////////////////////////////////////////////////////////////////////////
	void getVelocityFlattened(tuple3f & pos, int triangleIndex, tuple3f & result, 
		std::vector<float> & weight_buffer,bool useHarmonicField = true);


	void addForces2Vorticity(float timestep);

	void addDiffusion2Vorticity();

	//////////////////////////////////////////////////////////////////////////
	// calculate the harmonic flow for a bordered mesh, where
	// the constraints on the border are given by borderconstraints.
	// The constraints have to be understood as that the flow on the border
	// component i is constrained to be borderConstraints[i].
	// this will be added to the velocity field when pathtracing.
	//////////////////////////////////////////////////////////////////////////
	oneForm setHarmonicFlow(vector<tuple3f> & borderConstraints);

	//////////////////////////////////////////////////////////////////////////
	void adaptMatrices_zeroTotalBorderVort( vector<vector<int>> &brdr, pardisoMatrix &star0inv,pardisoMatrix &duald1 );

	oneForm & getHarmonicFlux();

	void updateVelocities();
	oneForm & getFlux();
	
//////////////////////////////////////////////////////////////////////////
	// ID IE IB IU IG   IS IT IU IF IF 
	//////////////////////////////////////////////////////////////////////////
	// Debug-Display Dual vertices
	//////////////////////////////////////////////////////////////////////////
	void showDualPositions();

	void showHarmonicField();

	void showVorticityPart();

	//////////////////////////////////////////////////////////////////////////
	// pathtrace and display pathtracedPositions
	// Again this is for debug / visualisation reasons
	//////////////////////////////////////////////////////////////////////////
	void pathTraceAndShow(float howmuch);
	void showFlux2Vel();


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

	bool checkAllDualVerticesInside();



/////////////////////////////////////////////////////////////////////////
// display the field
//////////////////////////////////////////////////////////////////////////
	void setStreamlines(bool on);
	void setStreamlineLength( int length );
	void setInterpolation(bool on);
	void glDisplayField();
	float texPos( int j, int nrPoints );
	float getFPS();
	float getSimTime();
	//helper method that returns a randomly chosen point on the triangle
	tuple3f randPoint( int triangle );
//////////////////////////////////////////////////////////////////////////
//colormap Methods
//////////////////////////////////////////////////////////////////////////
	virtual tuple3f color( int vertexNr );

	virtual std::string additionalInfo( void );
	void scrollAction(int what){};
	void actualizeFPS();


//////////////////////////////////////////////////////////////////////////
// debug/quality check
//////////////////////////////////////////////////////////////////////////
	void testFlux();

	//////////////////////////////////////////////////////////////////////////
	// Intern variable. Methods for debug/quality check.
	//////////////////////////////////////////////////////////////////////////
	vector<tuple3f> & getVelocities();
	vector<tuple3f> & getBacktracedVelocities();
	nullForm & getVorticity();
	std::vector<tuple3f> & getDualVertices();
	void showVorticity( bool param1 );
	void showTexLines( bool what );
	void setColorScale( float  scale );

};
