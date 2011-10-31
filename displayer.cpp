#include "displayer.h"
#include "mesh.h"

Displayer::Displayer(QWidget *parent)
	: QGLWidget(parent)
{
	resize(500,500);
	mode = EDGEMODE;
	this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

Displayer::~Displayer()
{

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
