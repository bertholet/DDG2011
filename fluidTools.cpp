#include "fluidTools.h"
#include "matrixf.h"

fluidTools::fluidTools(void)
{
}

fluidTools::~fluidTools(void)
{
}

void fluidTools::flux2Velocity( oneForm flux, std::vector<tuple3f> & bariCoords, std::vector<tuple3f> & target )
{
	assert(target.size() == flux.size());

	matrix3f flux2vel;
	matrix3f n_cross_T;
	//vertices of face
	tuple3f a,b,c;
	//triangle normal
	tuple3f n;
	//flux of actual face
	tuple3f flx;

	for(int i = 0; i < target.size(); i++){
		flux2vel.set(b.x-a.x,b.y-a.y,b.z-a.z,
			c.x-b.x,c.y-b.y,c.z-b.z,
			a.x-c.x,a.y-c.y,a.z-c.z);	
		n_cross_T.set(0,n.z,-n.y,
					-n.z,0,n.x,
					n.y,-n.x,0);
		flux2vel = (flux2vel*n_cross_T).inv();

		target[i].set(flux2vel * flx);
	}

	finish implement this!!!!!
}


