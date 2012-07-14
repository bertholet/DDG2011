/*
* Pardiso Header for Visual C++
*
* By Peter Bertholet (2011)
*/

/* PARDISO prototype. */
#pragma once
#include "pardisoMatrix.h"
#include <assert.h>
//#include <vector>
//#include "pardisoConfig.h"


extern "C" __declspec(dllimport) void pardisoinit (void   *, int    *,   int *, int *, double *, int *);
extern "C" __declspec(dllimport) void pardiso     (void   *, int    *,   int *, int *,    int *, int *, 
												   double *, int    *,    int *, int *,   int *, int *,
												   int *, double *, double *, int *, double *);
extern "C" __declspec(dllimport) void pardiso_chkmatrix  (int *, int *, double *, int *, int *, int *);
extern "C" __declspec(dllimport) void pardiso_chkvec     (int *, int *, double *, int *);
extern "C" __declspec(dllimport) void pardiso_printstats (int *, int *, double *, int *, int *, int *,
														  double *, int *);


/************************************************************************/
/* A wrapper class for the pardiso solver. Note: there are more
*  possible Attributes and configurations than accessible by this wrapper
*/
/************************************************************************/
class pardisoSolver{

private: 
	int int_params[128];
	double double_params[128];
	void* intern_memory[128];

	int matrix_type;
	int print_stats;
	int error;
	int nrhs;
	//static vector<pardisoConfig> configs;


	bool matrixWasSet;
//	static bool isInUse;
	//static bool keepMemory;

	void init_intParams(int nrRefinementSteps);
	void checkError_init();



public:
	//other matrix types do exist, see manual.
	static const int MT_ANY=11,MT_SYMMETRIC=-2,MT_STRUCTURALLY_SYMMETRIC=1;
	//solver types
	static const int SOLVER_DIRECT = 0, SOLVER_ITERATIVE=1;
	pardisoMatrix * matrix;

	pardisoSolver(
			int  matrix_typ, int solver,
			int nr_refinement_steps);


	~pardisoSolver(void);

	/************************************************************************/
	/* nr_righthandsides is the number of rows of b in Ax = b.   
	/* Note: setMatrix will factorize the matrix, you are allowed to set a Matrix
	/* only once per Solver. If the matrix is changed you need a new solver.
	/************************************************************************/
	void setMatrix(pardisoMatrix & mat, int nr_righthandsides);


	void setPrintStatistics(bool print);

	void setStoreResultInB(bool what);

	void solve(double *x, double * b);

	static void checkMatrix( int matrix_type, pardisoMatrix & mat );


};

