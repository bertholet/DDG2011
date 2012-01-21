#include "StdAfx.h"
#include "matrixf.h"
#include <math.h>

matrixf::matrixf(void)
{
	//buff = new arr_44();
}

matrixf::matrixf( float a11,float a12,float a13,float a14,
				 float a21,float a22,float a23,float a24,
				 float a31,float a32,float a33,float a34,
				 float a41,float a42,float a43,float a44 )
{
	//buff = new arr_44();
	mat[0][0] = a11;
	mat[0][1] = a12;
	mat[0][2] = a13;
	mat[0][3] = a14;
	mat[1][0] = a21;
	mat[1][1] = a22;
	mat[1][2] = a23;
	mat[1][3] = a24;
	mat[2][0] = a31;
	mat[2][1] = a32;
	mat[2][2] = a33;
	mat[2][3] = a34;
	mat[3][0] = a41;
	mat[3][1] = a42;
	mat[3][2] = a43;
	mat[3][3] = a44;
}

matrixf::matrixf( float arr[4][4] )
{
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			mat[i][j] = arr[i][j];
		}
	}
}

/*matrixf::matrixf( const matrixf& par )
{
	//buff = new arr_44();
	for (int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++){
			mat[i][j] = par.mat[i][j];
		}
	}
}*/

matrixf::~matrixf(void)
{
	//delete buff;
}

matrixf matrixf::operator*( matrixf &other )
{
	float temp[4][4];

	for(int i = 0; i < 4; i++){
		for(int j= 0; j < 4; j++){
			temp[i][j] = mat[i][0]*other.mat[0][j] +
				mat[i][1]*other.mat[1][j] +
				mat[i][2]*other.mat[2][j] +
				mat[i][3]*other.mat[3][j];
		}
	}
	return matrixf(temp);
}

tuple3f matrixf::operator*( tuple3f &other )
{
	return tuple3f(
		mat[0][0]* other.x + mat[1][0]* other.y+ mat[2][0]* other.z + mat[3][0],
		mat[0][1]* other.x + mat[1][1]* other.y+ mat[2][1]* other.z + mat[3][1],
		mat[0][2]* other.x + mat[1][2]* other.y+ mat[2][2]* other.z + mat[3][2]);
}

matrixf matrixf::operator*( float other )
{
	float temp[4][4];
	for (int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++){
			temp[i][j] = other * mat[i][j];
		}
	}
	return matrixf(temp);
}

matrixf matrixf::operator+( matrixf &other )
{	
	float temp[4][4];
	for (int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++){
			temp[i][j] = other.mat[i][j] + mat[i][j];
		}
	}
	return matrixf(temp);
}

matrixf matrixf::transpose( void )
{
	float temp[4][4];
	for (int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++){
			temp[i][j] = mat[j][i];
		}
	}
	return matrixf(temp);
}

matrixf matrixf::inv( void )
{
	float invert[4][4];
	float _det;
	_det = det();

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			invert[i][j] = inv_helper(i,j)/_det;
		}
	}
	
	return matrixf(invert);
}

float matrixf::det()
{

	float _det;
	_det = mat[0][0]*mat[1][1]*mat[2][2]*mat[3][3] +
		mat[0][0]*mat[1][2]*mat[2][3]*mat[3][1] +
		mat[0][0]*mat[1][3]*mat[2][1]*mat[3][2];

	_det+=	mat[0][1]*mat[1][0]*mat[2][3]*mat[3][2] +
		mat[0][1]*mat[1][2]*mat[2][0]*mat[3][3] +
		mat[0][1]*mat[1][3]*mat[2][2]*mat[3][0] ;

	_det+=	mat[0][2]*mat[1][0]*mat[2][1]*mat[3][3] +
		mat[0][2]*mat[1][1]*mat[2][3]*mat[3][0] +
		mat[0][2]*mat[1][3]*mat[2][0]*mat[3][1] ;

	_det+= mat[0][3]*mat[1][0]*mat[2][2]*mat[3][1] +
		mat[0][3]*mat[1][1]*mat[2][0]*mat[3][2] +
		mat[0][3]*mat[1][2]*mat[2][1]*mat[3][0] ;


	_det+=	-mat[0][0]*mat[1][1]*mat[2][3]*mat[3][2] -
		mat[0][0]*mat[1][2]*mat[2][1]*mat[3][3] -
		mat[0][0]*mat[1][3]*mat[2][2]*mat[3][1] ;

	_det+=	-mat[0][1]*mat[1][0]*mat[2][2]*mat[3][3] -
		mat[0][1]*mat[1][2]*mat[2][3]*mat[3][0] -
		mat[0][1]*mat[1][3]*mat[2][0]*mat[3][2] ;

	_det+=	-mat[0][2]*mat[1][0]*mat[2][3]*mat[3][0] -
		mat[0][2]*mat[1][1]*mat[2][0]*mat[3][3] -
		mat[0][2]*mat[1][3]*mat[2][1]*mat[3][0] ;

	_det+=	-mat[0][3]*mat[1][0]*mat[2][1]*mat[3][2] -
		mat[0][3]*mat[1][1]*mat[2][2]*mat[3][0] -
		mat[0][3]*mat[1][2]*mat[2][0]*mat[3][1];
	return _det;
}

float matrixf::inv_helper( int i, int j )
{
	float res=0;
	float res_factor;
	int ind_i[3];
	int ind_j[3];

	int ind = 0;
	for(int k = 0; k < 4; k++){
		if(k!=j){
			ind_i[ind] = k;
			ind++;
		}
	}

	ind = 0;
	for(int k = 0; k < 4; k++){
		if(k!=i){
			ind_j[ind] = k;
			ind++;
		}
	}

	for(int k = 0; k < 3; k++){
		res+= mat[ind_i[0]][ind_j[k]] * 
			mat[ind_i[1]][ind_j[(k+1)%3]] *
			mat[ind_i[2]][ind_j[(k+2)%3]];
		res-=mat[ind_i[0]][ind_j[k]] * 
			mat[ind_i[1]][ind_j[(k+2)%3]] *
			mat[ind_i[2]][ind_j[(k+1)%3]];
	}

	res *= pow(-1.f,i+j);

	return res;
}

//////////////////////////////////////////////////////////////////////////
// Matrix3f
//////////////////////////////////////////////////////////////////////////


matrix3f::matrix3f( void )
{

}

matrix3f::matrix3f( float a11, float a12, float a13, float a21, float a22, float a23, float a31, float a32, float a33 )
{
	mat[0][0] = a11;
	mat[0][1] = a12;
	mat[0][2] = a13;
	mat[1][0] = a21;
	mat[1][1] = a22;
	mat[1][2] = a23;
	mat[2][0] = a31;
	mat[2][1] = a32;
	mat[2][2] = a33;
}

matrix3f::matrix3f( float arr[3][3] )
{
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			mat[i][j] = arr[i][j];
		}
	}
}



tuple3f matrix3f::operator*( tuple3f &other )
{
	return tuple3f(
		mat[0][0]* other.x + mat[0][1]* other.y+ mat[0][2]* other.z ,
		mat[1][0]* other.x + mat[1][1]* other.y+ mat[1][2]* other.z,
		mat[2][0]* other.x + mat[2][1]* other.y+ mat[2][2]* other.z );
}

matrix3f matrix3f::operator*( matrix3f &other )
{
	float temp[3][3];
	for (int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++){
			temp[i][j] = mat[i][0]*other.mat[0][j] +
				mat[i][1]*other.mat[1][j] +
				mat[i][2]*other.mat[2][j];
		}
	}
	return matrix3f(temp);
}

matrix3f matrix3f::inv( void )
{
	float det_ = det();
	float temp[3][3];
	assert(det_ > 0.00001);
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			temp[i][j] = val(j,i) / det_;
		}
	}

	return matrix3f(temp);
}

float matrix3f::det()
{
	return mat[0][0]*mat[1][1]*mat[2][2]+mat[0][1]*mat[1][2]*mat[2][0] +
		mat[0][2]*mat[1][0]*mat[2][1] - mat[0][0]*mat[1][2]*mat[2][1] -
		mat[0][1]*mat[1][0]*mat[2][2] -mat[0][2]*mat[1][1]*mat[2][0];
}

float matrix3f::val( int i, int j )
{
	return mat[(i+1)%3][(j+1)%3]*mat[(i+2)%3][(j+2)%3] - 
		mat[(i+2)%3][(j+1)%3]*mat[(i+1)%3][(j+2)%3];
}

void matrix3f::set( float a11, float a12, float a13, float a21, float a22, float a23, float a31, float a32, float a33 )
{
	mat[0][0] = a11;
	mat[0][1] = a12;
	mat[0][2] = a13;
	mat[1][0] = a21;
	mat[1][1] = a22;
	mat[1][2] = a23;
	mat[2][0] = a31;
	mat[2][1] = a32;
	mat[2][2] = a33;
}

matrix3f::~matrix3f()
{

}

void matrix3f::setRow( int row, tuple3f & vals )
{
	mat[row][0] = vals.x;
	mat[row][1] = vals.y;
	mat[row][2] = vals.z;
}
