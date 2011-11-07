#pragma once
#include "GL/glew.h"
#include <QGLWidget>
#include <QMouseEvent>

class trackBallListener
{
	int a;
public:
	trackBallListener(QGLWidget * display);
	~trackBallListener(void);

	void onMouseMove(QMouseEvent* event );
	void onMousePress(QMouseEvent* event );

private:
	QGLWidget * displyer;
	float lastx, lasty, lastz;
};
