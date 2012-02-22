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
#include "curvVisualizingMesh.h"

class fluidControlWidget : public QWidget, public Observer<Model::modelMsg>, public Observer<borderMarkupMap*>
{
	Q_OBJECT

public:
	fluidControlWidget(QWidget *parent = 0);
	~fluidControlWidget();

	virtual void update( void * src, Model::modelMsg msg );
	virtual void update( void * src, borderMarkupMap * msg );

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
	void borderDirInput( const QString & text );


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

	//////////////////////////////////////////////////////////////////////////
	//border constraints.
	int selectedBorder;
	std::vector<tuple3f> borderConstrDirs;
	//tuple3f borderConstrDir;
};

#endif // FLUIDCONTROLWIDGET_H
