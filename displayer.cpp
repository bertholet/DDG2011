#include "displayer.h"
#include "mesh.h"
#include <math.h>
#include <QtGui/QMouseEvent>

Displayer::Displayer(QWidget *parent)
	: QGLWidget(parent)
{
	//resize(500,500);
	this->setMinimumHeight(300);
	this->setMinimumWidth(300);
	mode = EDGEMODE;
	this->map = NULL;
	this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

Displayer::~Displayer()
{
	if(this->map != NULL){
		delete map;
	}
}

void Displayer::initializeGL()
{
	glViewport(0, 0, this->width(), this->height());       
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0,GLdouble(this->width())/this->height(), 2.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);

}

void Displayer::paintGL()
{
	mesh * theMesh = Model::getModel()->getMesh();
	if(theMesh!=NULL){
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		if(mode == EDGEMODE){
			theMesh->glDisplayLines();
		}
		else if(mode == FLATMODE){
			theMesh->glDisplay();
		}
		else if(mode == COLORMAPMODE && map != NULL){
			theMesh->glDisplay(*map);
		}
		else{
			theMesh->glDisplay();
		}
		glFlush();
	}
	
}

void Displayer::resizeGL(int width, int height)
{
	glViewport(0, 0, this->width(), this->height());   
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0,GLdouble(this->width())/this->height(), 2.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);
}

void Displayer::setMode( DisplayMode aMode )
{
	this->mode = aMode;
	this->updateGL();
}

//////////////////////////////////////////////////////////////////////////
//the displayer will take care of freeing map
void Displayer::setColormap( colorMap * map )
{
	if(this->map != NULL){
		delete this->map;
	}
	this->map = map;
}

void Displayer::mouseMoveEvent( QMouseEvent* event )
{
	float x,y,z;
	float min = (this->width() > this->height()? this->height(): this->width());
	x= ((0.f -this->width())/2 +event->x())/(min/2);
	y = ((0.f +this->height())/2 - event->y())/(min/2);
	z = 1-x*x -y*y;
	z = (z < 0? 0: z);
	z = sqrtf(z);
	float nrm = sqrtf(x*x+y*y+z*z);
	x= x/nrm;
	y= y/nrm;
	z= z/nrm;

	float axisx=y*lastz-lasty*z,axisy=z*lastx-lastz*x,axisz=x*lasty-y*lastx;
	float axisnrm = sqrtf(axisx*axisx+axisy*axisy+axisz*axisz);
	float angle = acos(x*lastx+y*lasty+z*lastz);
	Model::getModel()->getMesh()->rot(angle,axisx/axisnrm,
		axisy/axisnrm,
		axisz/axisnrm);

	this->updateGL();

	lastx = x;
	lasty=y;
	lastz=z;

}

void Displayer::mousePressEvent( QMouseEvent* event )
{
	float min = (this->width() > this->height()? this->height(): this->width());
	lastx = ((0.f -this->width())/2+event->x()) /(min/2);
	lasty = ((0.f +this->height())/2 -event->y()) / (min/2);

	lastz = 1-lastx*lastx -lasty*lasty;
	lastz = (lastz < 0? 0: lastz);
	lastz = sqrtf(lastz); 

	float nrm = lastx*lastx + lasty*lasty+ lastz*lastz;
	nrm = sqrtf(nrm);
	lastx=lastx/nrm;
	lasty=lasty/nrm;
	lastz=lastz/nrm;
}
