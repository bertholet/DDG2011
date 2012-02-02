#include "trackBallListener.h"
#include <math.h>
#include "Model.h"

trackBallListener::trackBallListener(QGLWidget * display)
{
	this->displyer = display;
}

trackBallListener::~trackBallListener(void)
{
}

void trackBallListener::onMouseMove( QMouseEvent* event )
{
	float x,y,z;
	float min = (displyer->width() > displyer->height()? displyer->height(): displyer->width());
	x= ((0.f -displyer->width())/2 +event->x())/(min/2);
	y = ((0.f +displyer->height())/2 - event->y())/(min/2);
	z = 1-x*x -y*y;
	z = (z < 0? 0: z);
	z = sqrtf(z);
	assert(z==z);
	float nrm = x*x+y*y+z*z;
	nrm = (nrm < 0? 0: nrm);
	nrm = sqrtf(nrm);
	assert(nrm == nrm);
	x= x/nrm;
	y= y/nrm;
	z= z/nrm;

	float axisx=y*lastz-lasty*z,axisy=z*lastx-lastz*x,axisz=x*lasty-y*lastx;
	float axisnrm = axisx*axisx+axisy*axisy+axisz*axisz;
	axisnrm = (axisnrm< 0 ? 0: axisnrm);
	axisnrm = sqrtf(axisnrm);
	assert(axisnrm == axisnrm);
	float cos = x*lastx+y*lasty+z*lastz;
	cos = (cos<-1.f?-1.f :(cos>1?1.f:cos));

	float angle = acos(cos);
	assert(angle == angle);

	if(axisnrm < 0.001){
		return;
	}
	assert(axisnrm > 0);
	Model::getModel()->getMesh()->rot(angle,axisx/axisnrm,
		axisy/axisnrm,
		axisz/axisnrm);

	displyer->updateGL();

	lastx = x;
	lasty=y;
	lastz=z;
}

void trackBallListener::onMousePress( QMouseEvent* event )
{
	float min = (displyer->width() > displyer->height()? displyer->height(): displyer->width());
	lastx = ((0.f -displyer->width())/2+event->x()) /(min/2);
	lasty = ((0.f +displyer->height())/2 -event->y()) / (min/2);

	lastz = 1-lastx*lastx -lasty*lasty;
	lastz = (lastz < 0? 0: lastz);
	lastz = sqrtf(lastz); 

	float nrm = lastx*lastx + lasty*lasty+ lastz*lastz;
	nrm = sqrtf(nrm);
	lastx=lastx/nrm;
	lasty=lasty/nrm;
	lastz=lastz/nrm;
}
