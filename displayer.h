#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "Model.h"
#include <QGLWidget>
#include "curvVisualizingMesh.h"

enum DisplayMode {EDGEMODE,FLATMODE,COLORMAPMODE};

class Displayer : public QGLWidget
{
	Q_OBJECT

public:
	Displayer(QWidget *parent);
	~Displayer();

	void setMode(DisplayMode aMode);
	void setColormap(colorMap * map);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);

private:
	DisplayMode mode;
	colorMap * map;

	//for the trackball
	float lastx, lasty, lastz;
};

#endif // DISPLAYER_H
