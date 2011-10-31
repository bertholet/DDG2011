#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "Model.h"
#include <QGLWidget>

enum DisplayMode {EDGEMODE,FLATMODE,BORDERMODE};

class Displayer : public QGLWidget
{
	Q_OBJECT

public:
	Displayer(QWidget *parent);
	~Displayer();

	void setMode(DisplayMode aMode);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

private:
	DisplayMode mode;
};

#endif // DISPLAYER_H
