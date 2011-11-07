#pragma once
#include "matrixf.h"
#include <math.h>

class matrixFactory
{
public:
	matrixFactory(void);
	~matrixFactory(void);

	static matrixf id(void){
		return matrixf(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	}

	static matrixf translate(float dx, float dy, float dz){
		return matrixf(1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			dx,dy,dz,1);
	}

	static matrixf rotateX(float thetax){
		return matrixf(1,0,0,0,
			0,cos(thetax), -sin(thetax), 0,
			0, sin(thetax), cos(thetax), 0,
			0,0,0,1);
	}

	static matrixf rotateY(float thetay){
		return matrixf(cos(thetay),0,-sin(thetay),0,
			0,1, 0, 0,
			sin(thetay), 0, cos(thetay), 0,
			0,0,0,1);
	}

	static matrixf rotateZ(float thetaz){
		return matrixf(cos(thetaz),sin(thetaz),0,0,
			-sin(thetaz),cos(thetaz), 0, 0,
			0, 0, 1, 0,
			0,0,0,1);
	}

	// x,y,z denote the rotation axis.
	static matrixf rotate( float angle, float x, float y, float z ){
		float c = cos(angle);
		float s = sin(angle);
		return matrixf(x*x*(1-c)+c,x*y*(1-c)-z*s,x*z*(1-c)+y*s,0,
			y*x*(1-c)+z*s,y*y*(1-c)+c, y*z*(1-c)-x*s, 0,
			x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z*(1-c)+c, 0,
			0,0,0,1);
	}
	static matrixf scale( float scal ){
		return matrixf(scal,0,0,0,0,scal,0,0,0,0,scal,0,0,0,0,1);
	}
};
