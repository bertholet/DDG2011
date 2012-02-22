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
#include <QLineEdit>

class fluidControlWidget : public QWidget, public Observer<Model::modelMsg>
{
	Q_OBJECT

public:
	fluidControlWidget(QWidget *parent = 0);
	~fluidControlWidget();

	virtual void update( void * src, Model::modelMsg msg );

	float getViscosity();
	float getForceStrength();
	float getTimestep();
	void updateViscTimeLabel();
	void updateAnimationLabel(float time, float fps);

public slots:
	void flux2vort2flux();
	void getCollectedFlux();
	void singleSimulationStep();
	void updateTimeStep();
	void setForceFlux();

	void initSimulation();

	void updateViscosity();
	void forceAgeChanged();
	void startSim();
	void doAnimation();
	void forceStrengthChanged();

private:
	std::vector<tuple3f> dirs;
	fluidSimulation * mySimulation;
	QSlider * stepSlider;
	QSlider * viscositySlider;
	QSlider * forceAgeSlider;
	QSlider * forceStrengthSlider;
	float stepSize;
	QTimer * animationTimer;
	QLabel * viscosityAndTimestep;
	QLabel * animationLabel;
	QLabel * forceAgeLabel;
	QLabel * forceStrengthLabel;
	QLineEdit * vectorInput;

	float forceAge;
	float maxForceAge;
	bool dirs_cleared;
};

#endif // FLUIDCONTROLWIDGET_H
