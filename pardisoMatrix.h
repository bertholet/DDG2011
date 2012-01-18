#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <string>
#include "pardisoMatCreator.h"
#include "mystatusbar.h"


class pardisoMatrix
{
private:
	//indices of the values in the sparse matrix
	std::vector<int> ia, ja;
	//values of the elements of the sparse matrix
	std::vector<double> a;

	int n, m;

public:
	pardisoMatrix(void);
	//////////////////////////////////////////////////////////////////////////
	// vals are copied. a is assumed to have the same size as ja. Do not forget
	// that the last element of i_a has to be #values +1 and thus has #rows + 1
	// values.
	//////////////////////////////////////////////////////////////////////////
	pardisoMatrix(int * ia_, int * ja_,double * a_,
		 int sz_ia, int sz_ja);
	~pardisoMatrix(void);


	//////////////////////////////////////////////////////////////////////////
	// creator describes the matrix in 0 based notation
	//////////////////////////////////////////////////////////////////////////
	void initMatrix(pardisoMatCreator & creator, int dim, myStatusBar * bar = NULL);


	//////////////////////////////////////////////////////////////////////////
	// clear all data stored
	//////////////////////////////////////////////////////////////////////////
	void clear();

	//////////////////////////////////////////////////////////////////////////
	// return first dim of this matrix.
	//////////////////////////////////////////////////////////////////////////
	int dim();

	//////////////////////////////////////////////////////////////////////////
	// getter
	//////////////////////////////////////////////////////////////////////////
	std::vector<int> & getia(void){
		return ia;
	}
	std::vector<int> & getja(void){
		return ja;
	}
	std::vector<double> & geta(void){
		return a;
	}
	int getn(){
		return n;
	}
	int getm(){
		return m;
	}

	//////////////////////////////////////////////////////////////////////////
	// psuhback with adaptation of dimension variables
	//////////////////////////////////////////////////////////////////////////
	void iapush_back(int i){
		ia.push_back(i);
		n = ia.size()-1;
	}
	void japush_back(int j){
		ja.push_back(j);
		if(j > m+1){
			m = j-1;
		}
	}
	void apush_back(double val){
		a.push_back(val);
	}
	

	//////////////////////////////////////////////////////////////////////////
	// store Matrix in matlab executable format in a file.
	// Handy for debugging and analysis.
	//////////////////////////////////////////////////////////////////////////
	void saveMatrix(std::string file);
	void saveVector(std::vector<double> & vctor, std::string  name, 
		std::string  file );

	//////////////////////////////////////////////////////////////////////////
	// Stores the indices i in target, such that a[i] is a value on the diagonal
	// Usefull e.g if you want to add epsilon to the diagonal.
	//////////////////////////////////////////////////////////////////////////
	void getDiagonalIndices( std::vector<int> & target_ind );

	//////////////////////////////////////////////////////////////////////////
	// IF and only IF mat has an entry at i,j, val is added.
	// i, j in ZERO based notation
	// ELSE an assertion will fail and an error be thrown
	//////////////////////////////////////////////////////////////////////////
	void add( int i, int j, float val );


	//////////////////////////////////////////////////////////////////////////
	// This method will replace all values in this Matrix by 1/val, if the
	// absolute values is > eps
	// For diagonalmatrices (only!) this gives the inverse.
	// Convenience Method
	//////////////////////////////////////////////////////////////////////////
	void elementWiseInv(double eps);


	//////////////////////////////////////////////////////////////////////////
	// Matrix multiplication. Note that even it is handy to use a temporary
	// sparse matrix is created and returned BY VALUE. 
	// Note also: this works only for Matrices that are fully stored, i.e. symmetricity is not used 
	// by storing only the upper half of the matrix
	//////////////////////////////////////////////////////////////////////////
	pardisoMatrix operator*(pardisoMatrix & other);

	//////////////////////////////////////////////////////////////////////////
	// Matrix multiplication with transposed matrix. Returned by value, so
	// mediocrely efficient. Use for onetime construction of reused matrix
	// where efficiency is not as important.
	//////////////////////////////////////////////////////////////////////////
	pardisoMatrix operator % (pardisoMatrix & other);

	//////////////////////////////////////////////////////////////////////////
	// addition
	//////////////////////////////////////////////////////////////////////////
	pardisoMatrix operator + (pardisoMatrix & other);

};
