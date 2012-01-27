#ifndef FLUIDCONTROLWIDGET_H
#define FLUIDCONTROLWIDGET_H
#pragma  once

#include <QWidget>
#include "Observer.h"
#include "Model.h"
#include "fluidSimulation.h"

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

private:
	std::vector<tuple3f> dualVertices;
	fluidSimulation * mySimulation;
};

#endif // FLUIDCONTROLWIDGET_H
