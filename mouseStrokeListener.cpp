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
	//speed and position.
}

void mouseStrokeListener::onMousePress( QMouseEvent* event )
{
	matrixf proj, model;
	//glGetFloatv(GL_PROJECTION, (GLfloat *) &proj);
	float height = 1.73205081f; //tan(60)
	float width = (0.f +daddy->width()) / daddy->height() * height;

	tuple3f ray = tuple3f(event->x(), event->y(), 1);
	ray.x = (ray.x - daddy->width()/2) / daddy->width() * width;
	ray.y = (ray.y - daddy->width()/2) / daddy->width() * height;
	//assert aspect etc as induced by the 

	tuple3i * fc = Model::getModel()->getMesh()->intersect(ray);

	nrCalls++;
	this->map->mark(*fc, nrCalls);

	this->daddy->updateGL();
}
