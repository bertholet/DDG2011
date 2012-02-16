#ifndef FLUIDCONTROLWIDGET_H
#define FLUIDCONTROLWIDGET_H
#pragma  once

#include <QWidget>
#include "Observer.h"
#include "Model.h"
#include "fluidSimulation.h"
#include <QSlider>
#include <QLabel>
#include <qtimer.h>

class fluidControlWidget : public QWidget, public Observer<Model::modelMsg>
{
	Q_OBJECT

public:
	fluidControlWidget(QWidget *parent = 0);
	~fluidControlWidget();

	virtual void update( void * src, Model::modelMsg msg );

	float getViscosity();
	float getTimestep();
	void updateViscTimeLabel();

public slots:
	void flux2vort2flux();
	void getCollectedFlux();
	void singleSimulationStep();
	void stepSizeChanged();
	void setForceFlux();

	void initSimulation();

	void viscosityChanged();
	void startSim();
	void doAnimation();
private:
	std::vector<tuple3f> dirs;
	fluidSimulation * mySimulation;
	QSlider * stepSlider;
	QSlider * viscositySlider;
	float stepSize;
	QTimer * animationTimer;
	QLabel * viscosityAndTimestep;

	float forceAge;
	bool dirs_cleared;
};

#endif // FLUIDCONTROLWIDGET_H
