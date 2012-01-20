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
	mouseMode = TRACKBALLMODE;

	this->map = NULL;
	this->tmmap = new triangleMarkupMap();
	this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	displayVField = true;
	normedVField = true;
	displayPointCloud = true;
	tBallListener = new trackBallListener(this);
	strokeListener = new mouseStrokeListener(tmmap, this);
}

Displayer::~Displayer()
{
	if(this->map != NULL){
		delete map;
	}
	delete tBallListener;
	delete strokeListener;
	delete tmmap;
}

void Displayer::initializeGL()
{
	glViewport(0, 0, this->width(), this->height());       
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0,GLdouble(this->width())/this->height(), 1.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(3.0);
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
		else if(mode == MOUSEINPUTMODE && tmmap != NULL){
			theMesh->glDisplay((colorMap &) *tmmap);
			Model::getModel()->getInputCollector().glOutputConstraints(theMesh);
		}
		else{
			theMesh->glDisplay();
		}

		if(displayVField && Model::getModel()->getVField() != NULL){
			Model::getModel()->getVField()->glOutputField(normedVField);
		}

		if(displayPointCloud && Model::getModel()->getPointCloud() != NULL){
			std::vector<tuple3f> & points = * Model::getModel()->getPointCloud();
			glBegin(GL_POINTS);
			for(int i =0; i< points.size(); i++){
				glVertex3f(points[i].x,points[i].y,points[i].z);
			}
			glEnd();
		}
		glFlush();
	}
	
}

void Displayer::resizeGL(int width, int height)
{
	glViewport(0, 0, this->width(), this->height());   
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0,GLdouble(this->width())/this->height(), 1.0, 10000.0);
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
	if(mouseMode == TRACKBALLMODE){
		this->tBallListener->onMouseMove(event);
	}
	else if(mouseMode == INPUTMODE){
		this->strokeListener->onMouseMove(event);
	}
}

void Displayer::mousePressEvent( QMouseEvent* event )
{
	if(mouseMode == TRACKBALLMODE){
		this->tBallListener->onMousePress(event);
	}
	else if(mouseMode == INPUTMODE){
		this->strokeListener->onMousePress(event);
	}
}

void Displayer::setMouseMode( MouseInputMode aMode )
{
	this->mouseMode = aMode;
}

void Displayer::resetStrokes()
{
	this->tmmap->reset();
	updateGL();
}

void Displayer::setNormedFieldDisplay( bool what )
{
	this->normedVField = what;
}

void Displayer::setLineWidth( float param1 )
{
	glLineWidth(param1);
}

void Displayer::wheelEvent( QWheelEvent* ev )
{
	Model::getModel()->getMesh()->move((0.f + ev->delta())/800);
	updateGL();
}

void Displayer::setPointCloudDisplay( bool what)
{
	this->displayPointCloud = what;
}
