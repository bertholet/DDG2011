#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "Model.h"
#include <QGLWidget>
#include "curvVisualizingMesh.h"
#include "trackBallListener.h"
#include "mouseStrokeListener.h"


enum DisplayMode {EDGEMODE,FLATMODE,COLORMAPMODE, MOUSEINPUTMODE};
enum MouseInputMode {TRACKBALLMODE,INPUTMODE};

class Displayer : public QGLWidget
{
	Q_OBJECT

public:
	Displayer(QWidget *parent);
	~Displayer();

	void setMode(DisplayMode aMode);
	void setMouseMode(MouseInputMode aMode);
	void setColormap(colorMap * map);
	void setNormedFieldDisplay(bool);
	void resetStrokes();
	void setLineWidth( float param1 );

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);

private:
	DisplayMode mode;
	MouseInputMode mouseMode;
	colorMap * map;
	triangleMarkupMap * tmmap;

	bool normedVField;
	bool displayVField;
	trackBallListener * tBallListener;
	mouseStrokeListener * strokeListener;
	//for the trackball
//	float lastx, lasty, lastz;
};

#endif // DISPLAYER_H
