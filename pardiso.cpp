#include "StdAfx.h"
#include "pardiso.h"
#include <Windows.h>

//bool pardisoSolver::isInUse = false;
//bool pardisoSolver::keepMemory = true;

void pardisoSolver::init_intParams(int nrRefinementSteps)
{
	SYSTEM_INFO sysinfo; 	GetSystemInfo( &sysinfo );  	
	int num_procs = sysinfo.dwNumberOfProcessors;    
	int_params[0] = 0; //use default params..
	int_params[2]  = num_procs;

	//all parameters have to be set after init!. (but for param 2 = num_procs)
	//int_params[3] = 91;	//hmhmhm
	int_params[7] = nrRefinementSteps;//1       /* Max numbers of iterative refinement steps. */
	int_params[6] = 0;	//int_params[6] = 1; then the result is placed in b.
	int_params[32] = 1; // compute determinant 
}

pardisoSolver::pardisoSolver( int matrix_typ, int solver, int nr_refinement_steps )
{
	//assert(!isInUse ||keepMemory);
	error = 0;
	matrix = NULL; 
	matrix_type = matrix_typ;
	print_stats = 0;
	init_intParams(nr_refinement_steps);
	pardisoinit(intern_memory, &matrix_type, &solver, int_params, double_params, &error);
	checkError_init();

	matrixWasSet = false;
//	isInUse = true;
	//keepMemory = false;
}

pardisoSolver::~pardisoSolver( void )
{
	int phase = -1;
/*	if(keepMemory){
		phase = 0; //release only memory of this factorization
	}*/

	int maxfct =1; /*max nr of factorizations*/
	int mnum =1; /* Which factorization to use. */
	int n = matrix->dim();

	if(matrix->geta().size() > 0){
		int idumm;
		pardiso (intern_memory, &maxfct, &mnum, &matrix_type, &phase,
			&n, &matrix->geta()[0], &matrix->getia()[0], &matrix->getja()[0], &idumm, &nrhs,
			int_params, &print_stats, NULL, NULL, &error, double_params);

	}
	else{
		double ddumm;
		int idumm;
		pardiso (intern_memory, &maxfct, &mnum, &matrix_type, &phase,
			&n, &ddumm, &matrix->getia()[0], &idumm, &idumm, &nrhs,
			int_params, &print_stats, &ddumm, &ddumm, &error, double_params);
	}

	//	isInUse = false;

	if(error != 0){
		throw std::runtime_error("Exception in pardiso solve-");
	}
}

void pardisoSolver::setMatrix( pardisoMatrix & mat, int nr_righthandsides )
{
	assert(!matrixWasSet);
	assert(mat.getn() == mat.getm());
	matrix = &mat;
	nrhs = nr_righthandsides;
	if(mat.geta().size() > 0){
		checkMatrix(matrix_type, mat);

		//factorization: symbolic and numerical
		int phase = 12;
		int maxfct =1; /*max nr of factorizations*/
		int mnum =1; /* Which factorization to use. */
		int n = mat.dim();

		pardiso (intern_memory, &maxfct, &mnum, &matrix_type, &phase,
			&n, &mat.geta()[0], &mat.getia()[0], &mat.getja()[0], NULL, &nr_righthandsides,
			int_params, &print_stats, NULL, NULL, &error, double_params);
	}
	else{
		//factorization: symbolic and numerical
		int phase = 12;
		int maxfct =1; /*max nr of factorizations*/
		int mnum =1; /* Which factorization to use. */
		int n = mat.dim();
		double ddumm;
		int idumm;
		pardiso (intern_memory, &maxfct, &mnum, &matrix_type, &phase,
			&n, &ddumm, &mat.getia()[0], &idumm, NULL, &nr_righthandsides,
			int_params, &print_stats, NULL, NULL, &error, double_params);
	}
	if(error != 0){
		checkMatrix(this->matrix_type, mat);
		throw std::runtime_error("Exception in pardiso solve-");
	}

	matrixWasSet = true;
}

void pardisoSolver::setPrintStatistics( bool print )
{
	if(print){
		print_stats = 1;
	}
	else{
		print_stats =0;
	}
}

void pardisoSolver::setStoreResultInB( bool what )
{
	if(what == true){
		int_params[5] = 1;
	}
	else{
		int_params[5] = 0;
	}
}

void pardisoSolver::solve( double *x, double * b )
{
	assert(matrix->getn() == matrix->getm());
	int phase = 33;
	int maxfct =1; /*max nr of factorizations*/
	int mnum =1; /* Which factorization to use. */
	int n = matrix->dim();

	error = 0;
	pardiso (intern_memory, &maxfct, &mnum, &matrix_type, &phase,
		&n, &matrix->geta()[0], &matrix->getia()[0], &matrix->getja()[0], NULL, &nrhs,
		int_params, &print_stats, b, x, &error, double_params);

	if(error != 0){
		throw std::runtime_error("Exception in pardiso solve-");
	}
}

void pardisoSolver::checkMatrix( int matrix_type, pardisoMatrix & mat )
{
	int n = mat.dim();
	int err;
	pardiso_chkmatrix  (&matrix_type, &n, & mat.geta()[0], & mat.getia()[0], 
		& mat.getja()[0], &err);

	if (err != 0) {
		printf("\nERROR in consistency of matrix: %d", err);

		throw std::runtime_error("Pardiso::checkmatrix error");
	}
}

void pardisoSolver::checkError_init()
{
	//error = 0;
	if (error != 0) 
	{
		if (error == -10 )
			printf("No license file found \n");
		else if (error == -11 )
			printf("License is expired \n");
		else if (error == -12 )
			printf("Wrong username or hostname \n");
		else
			printf("Error %d has occurred\n", error);

	}
	else
		printf("[PARDISO]: License check was successful ... \n");
}
