#pragma once
#include "GL/glew.h"
#include <QGLWidget>
#include <QMouseEvent>
#include "curvVisualizingMesh.h"
#include <GL/GLU.h>

class mouseStrokeListener
{
private:
	triangleMarkupMap * map;
	QGLWidget *daddy;
	int nrCalls;

	tuple3f last;
	bool lastValid;
public:
	mouseStrokeListener(triangleMarkupMap * _map, QGLWidget *daddy);
	~mouseStrokeListener(void);

	void onMouseMove(QMouseEvent* event );
	void onMousePress(QMouseEvent* event );
};
