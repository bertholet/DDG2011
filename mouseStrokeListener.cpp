#include "mouseStrokeListener.h"
#include "matrixf.h"
#include "Model.h"

mouseStrokeListener::mouseStrokeListener(triangleMarkupMap * _map, QGLWidget *_daddy)
{
	this->map = _map;
	this->daddy = _daddy;
	this->nrCalls = 0;
}

mouseStrokeListener::~mouseStrokeListener(void)
{
}

void mouseStrokeListener::onMouseMove( QMouseEvent* event )
{
	GLdouble proj[16], model[16], Z;
	GLint viewPort[4];
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, viewPort);

	float winx = (float) event->x() , 
		winy = (float) (viewPort[3] - event->y()), winz;

	/*glReadPixels(winy, winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Z );
	winz = (float) Z;*/

	GLdouble p1x,p1y,p1z;
	tuple3f start, end;

	gluUnProject(winx, winy,-1, model,proj,viewPort,&p1x,&p1y,&p1z);
	start.x = (float) p1x;
	start.y = (float) p1y;
	start.z = (float) p1z;

	gluUnProject(winx, winy,0, model,proj,viewPort,&p1x,&p1y,&p1z);
	end.x = (float) p1x;
	end.y = (float) p1y;
	end.z = (float) p1z;


	int vertex, face;
	tuple3f next;
	tuple3i * fc = Model::getModel()->getMesh()->intersect(start,end, &vertex, &face, next);

	if(fc != NULL){
		//this->map->mark(*fc, nrCalls);
		this->map->mark(vertex, nrCalls);
		this->daddy->updateGL();

		Model::getModel()->getInputCollector().collect(vertex);

		if(lastValid){
			Model::getModel()->getInputCollector().collect(face, next - last);
		}
		last.set(next);
		lastValid = true;
	}
	else{
		lastValid = false;
	}
}

void mouseStrokeListener::onMousePress( QMouseEvent* event )
{
	nrCalls++;
	lastValid = false;
	onMouseMove(event);

}
