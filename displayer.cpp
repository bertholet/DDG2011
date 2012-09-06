#include "displayer.h"
#include "mesh.h"
#include <math.h>
#include <QtGui/QMouseEvent>
#include "fluidSimulation.h"

Displayer::Displayer(QWidget *parent)
	: QGLWidget(parent)
{
	//resize(500,500);
	this->setMinimumHeight(300);
	this->setMinimumWidth(300);
	mode = EDGEMODE;
	mouseMode = TRACKBALLMODE;

	this->tex = new squareTexture();

	this->map = NULL;
	this->tmmap = new triangleMarkupMap();
	this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	Model::getModel()->attach(this);

	displayVField = true;
	normedVField = true;
	displayVectors = true;
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
	delete tex;
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

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	//antialias
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POLYGON_SMOOTH);
//	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);


	//bg color
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	//////////////////////////////////////////////////////////////////////////
	// textures for fluid sim
	//////////////////////////////////////////////////////////////////////////
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_1D, tex_id);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	GLfloat Texture4[4][3] =
	{

	//	{ 0.f, 0.f, 0.f },
		{ 0.1f, 0.1f, 0.1f },
		{ 0.8f,0.8f,0.8f }, // Blue
		{ 0.8f, 0.8f, 0.8f }, // Green
		{ 0.1f, 0.1f, 0.1f },
	/*	{ 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f },
		{ 1, 1, 1 }, // Blue
		{ 1, 1, 1 }, // Green
		{ 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f }*/
	};


	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 4 ,0,GL_RGB, GL_FLOAT, 
		Texture4);

	//Load 2dTexture 
	//GLuint tex_id;
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->szx,tex->szy,0,GL_RGBA, GL_FLOAT, 
		&(tex->checkboard[0]));
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	//glEnable(GL_TEXTURE_1D);

	//////////////////////////////////////////////////////////////////////////

	glLineWidth(3.0);
	glPointSize(3.f);
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
			theMesh->glDisplay(*map, smoothShading);
		}
		else if(mode == MOUSEINPUTMODE && tmmap != NULL){
			theMesh->glDisplay((colorMap &) *tmmap, smoothShading);
			Model::getModel()->getInputCollector().glOutputConstraints(theMesh);
		}
		else if(mode == FLUIDSIMMODE){
			fluidSimulation * sim = Model::getModel()->getFluidSimulation();
			if(sim != NULL){
				theMesh->glDisplay(*sim, smoothShading);
				sim->glDisplayField();
			}
			else{
				theMesh->glDisplay();
			}
			glFlush();
			return;
		}
		else if(mode == TEXMODE && theMesh->getTexCoords().size()>0 ){
			glEnable(GL_TEXTURE_2D);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			theMesh->glTexDisplay();
			glDisable(GL_TEXTURE_2D);
		}
		else if(mode == TEXMODE2 && theMesh->getTexCoords().size()>0 ){
			glEnable(GL_TEXTURE_2D);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			theMesh->glTexMapDisplay(Model::getModel()->getMeshInfo()->getBorder());
			glDisable(GL_TEXTURE_2D);
		}
		else if(mode == TEXMODE3 && theMesh->getTexCoords().size()>0 ){
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			theMesh->glTexEmbedDisplay(Model::getModel()->getMeshInfo()->getBorder());
		}
		else{
			theMesh->glDisplay();
		}


		if(displayVField && Model::getModel()->getVField() != NULL){
			Model::getModel()->getVField()->glOutputField(normedVField,Model::getModel()->getDisplayLength());
		}

		if(displayPointCloud && Model::getModel()->getPointCloud() != NULL){
			std::vector<tuple3f> & points = * Model::getModel()->getPointCloud();
			std::vector<tuple3f> & fcnormals=  Model::getModel()->getMesh()->getFaceNormals();

			glColor3f(0,0,1);
			glBegin(GL_POINTS);
			for(int i =0; i< points.size(); i++){
				glVertex3f(points[i].x,points[i].y,points[i].z);
			}
			glEnd();

			std::vector<std::vector<int>> & nbrFaces =  * Model::getModel()->getPointCloudConnectivity();
			tuple3f position;
			for(int i =0; i< nbrFaces.size(); i++){
				glBegin(GL_LINE_LOOP);
				for(int j =0; j< nbrFaces[i].size(); j++){
					position = points[nbrFaces[i][j]];
					if( nbrFaces[i][j] < fcnormals.size()){
						position += fcnormals[nbrFaces[i][j]] *0.001;
					}
					glVertex3fv((GLfloat *) & position);
				}
				glEnd();
			}
		}
		if(displayVectors && Model::getModel()->getPos() != NULL && Model::getModel()->getDirs() != NULL){
			std::vector<tuple3f> & pos= * Model::getModel()->getPos();
			std::vector<tuple3f> & dirs= * Model::getModel()->getDirs();
			std::vector<tuple3f> & fcnormals=  Model::getModel()->getMesh()->getFaceNormals();
			bool showArrows = Model::getModel()->getShowArrows();

			tuple3f normeddir, point, position;
			for(int i =0; i< fcnormals.size(); i++){
				glBegin(GL_LINE_LOOP);		
				glColor3f(0,0,0);
				position = pos[i] + fcnormals[i]*0.001;
				glVertex3fv((GLfloat *) & position);
				normeddir = dirs[i];
				if(normedVField){
					normeddir.normalize();
				}
				normeddir*= Model::getModel()->getDisplayLength();
				glColor3f(1,0,0);
				position = pos[i] + normeddir;
				glVertex3fv((GLfloat *) & position);
				glEnd();

				if(showArrows){
					glBegin(GL_TRIANGLES);
					glColor3f(0.5f,0,0);

					position = position + fcnormals[i]*0.001;
					glVertex3fv((GLfloat *) & position) ;
					point = fcnormals[i].cross(normeddir) *(0.15f) ;
					position = position - normeddir *0.3f;
					position = position + point;
					glVertex3fv((GLfloat *) & position) ;
					position = position + point *(-2.f);
					glVertex3fv((GLfloat *) & position) ;
					glEnd();
				}
			}
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
	if(mode == COLORMAPMODE && mouseMode == COLORMAPSCROLL){
		this->map->scrollAction(ev->delta());
	}
	else{
		Model::getModel()->getMesh()->move((0.f + ev->delta())/800);
	}
	updateGL();
}

void Displayer::setPointCloudDisplay( bool what)
{
	this->displayPointCloud = what;
}

void Displayer::update( void * src, Model::modelMsg msg )
{
	if(msg == Model::DISPLAY_CHANGED){
		updateGL();
	}
}

void Displayer::setVectorDisplay( bool )
{
	this->displayVectors = true;
	updateGL();
}

void Displayer::setSmooth( bool param1 )
{
	this->smoothShading = param1;
}
