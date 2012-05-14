#include "smoothingWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <string>
#include <sstream>
#include "Model.h"
#include "mesh.h"
#include <math.h>

smoothingWidget::smoothingWidget(QWidget *parent):QWidget(parent)
{
	implicitSmoother = NULL;
	timeStep = 0.001f; 
	smootherTimer = new QTimer(this);
	connect( smootherTimer, SIGNAL(timeout()), this, SLOT(doSmoothing()) );
	implicitSmootherTimer = new QTimer(this);
	connect( implicitSmootherTimer, SIGNAL(timeout()), this, SLOT(doImplicitSmoothing()) );


	QPushButton * smoothButton = new QPushButton("Explicit Smoothing");
	connect(smoothButton,SIGNAL(released()), this, SLOT(startDirectSmoothing()));

	QPushButton * implicitSmoothButton = new QPushButton("Implicit Smoothing");
	connect(implicitSmoothButton,SIGNAL(released()), this, SLOT(startImplicitSmoothing()));
	
	timeLabel = new QLabel("Timestep ()");
	
	timeStepSlider = new QSlider(Qt::Horizontal, this);
	timeStepSlider->setMinimum(1);
	timeStepSlider->setMaximum(1000);
	timeStepSlider->setTickPosition(QSlider::TicksAbove);
	connect(timeStepSlider,SIGNAL(sliderReleased()), this, SLOT(updateTimeStep()));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(timeLabel);
	layout->addWidget(timeStepSlider);
	layout->addWidget(smoothButton);
	layout->addWidget(implicitSmoothButton);

	this->setLayout(layout);
}

smoothingWidget::~smoothingWidget(void)
{
}

void smoothingWidget::updateTimeStep()
{
	this->timeStep = pow(10.f,10*(0.f + this->timeStepSlider->value()) /this->timeStepSlider->maximum() -5);
	std::stringstream ss;
	ss << "Timestep (" << this->timeStep << "):";
	this->timeLabel->setText(ss.str().c_str());
	smoother.setdt(timeStep);
}

void smoothingWidget::startDirectSmoothing()
{
	if(smootherTimer->isActive()){
		smootherTimer->stop();
	}
	else{
		smootherTimer->start(10);
	}
}

void smoothingWidget::doSmoothing()
{
	smoother.smootheMesh_explicitEuler(*(Model::getModel()->getMesh()));
	//Model::getModel()->getMesh()->updateObserver(meshMsg::POS_CHANGED);
	Model::getModel()->updateObserver(Model::DISPLAY_CHANGED);
}

void smoothingWidget::startImplicitSmoothing()
{

	if(implicitSmootherTimer->isActive()){
		implicitSmootherTimer->stop();
		delete implicitSmoother;
		implicitSmoother = NULL;
	}
	else{
		assert(implicitSmoother == NULL);
		implicitSmoother = new ImplicitEulerSmoothing(*(Model::getModel()->getMesh()),1,timeStep);
		implicitSmootherTimer->start(10);
	}
}

void smoothingWidget::doImplicitSmoothing()
{
	assert(implicitSmoother != NULL);
	implicitSmoother->smootheMesh(*(Model::getModel()->getMesh()));
		Model::getModel()->updateObserver(Model::DISPLAY_CHANGED);
}
