#ifndef FLUIDCONTROLWIDGET_H
#define FLUIDCONTROLWIDGET_H
#pragma  once

#include <QWidget>
#include "Observer.h"
#include "Model.h"
#include "fluidSimulation.h"
#include <QSlider>

class fluidControlWidget : public QWidget, public Observer<Model::modelMsg>
{
	Q_OBJECT

public:
	fluidControlWidget(QWidget *parent = 0);
	~fluidControlWidget();

	virtual void update( void * src, Model::modelMsg msg );

public slots:
	void circumcenters();
	void flux2Vel();
	void newFluidSim();
	void stepSizeChanged();

private:
	std::vector<tuple3f> dualVertices;
	fluidSimulation * mySimulation;
	QSlider * stepSlider;
	float stepSize;
};

#endif // FLUIDCONTROLWIDGET_H
