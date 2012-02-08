#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "Model.h"
#include <QGLWidget>
#include "curvVisualizingMesh.h"
#include "trackBallListener.h"
#include "mouseStrokeListener.h"
#include "Observer.h"
#include "Model.h"

enum DisplayMode {EDGEMODE,FLATMODE,COLORMAPMODE, MOUSEINPUTMODE, FLUIDSIMMODE};
enum MouseInputMode {TRACKBALLMODE,INPUTMODE};

class Displayer : public QGLWidget,public Observer<Model::modelMsg>
{
	Q_OBJECT

public:
	Displayer(QWidget *parent);
	~Displayer();

	void setMode(DisplayMode aMode);
	void setMouseMode(MouseInputMode aMode);
	void setColormap(colorMap * map);
	void setNormedFieldDisplay(bool);
	void setPointCloudDisplay(bool);
	void setVectorDisplay(bool);
	void resetStrokes();
	void setLineWidth( float param1 );
	void update(void * src, Model::modelMsg msg);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* ev);

private:
	DisplayMode mode;
	MouseInputMode mouseMode;
	colorMap * map;
	triangleMarkupMap * tmmap;

	bool normedVField;
	bool displayVField;
	bool displayPointCloud;
	trackBallListener * tBallListener;
	mouseStrokeListener * strokeListener;
	bool displayVectors;
	//for the trackball
//	float lastx, lasty, lastz;
};

#endif // DISPLAYER_H
