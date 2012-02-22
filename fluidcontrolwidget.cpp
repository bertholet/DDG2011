#include "fluidcontrolwidget.h"
#include <QPushButton>
#include "QVBoxLayout"
#include "meshMetaInfo.h"
#include "dualMeshTools.h"
#include "fluidTools.h"
#include "oneForm.h"
#include <QLabel>
#include <string>
#include <sstream>
#include <math.h>


/*#include "mesh.h"
#include "Operator.h"*/

fluidControlWidget::fluidControlWidget(QWidget *parent)
	: QWidget(parent)
{

	mySimulation = NULL;

	animationTimer = new QTimer(this);
	connect( animationTimer, SIGNAL(timeout()), this, SLOT(doAnimation()) ); 

	//setUp Gui

	QPushButton * butt = new QPushButton("Flux 2 Vorticity 2 Flux!");
	connect(butt, SIGNAL(released()), this, SLOT(flux2vort2flux()));
	QPushButton * butt2 = new QPushButton("Define Flux!");
	connect(butt2, SIGNAL(released()), this, SLOT(getCollectedFlux()));
	QPushButton * butt_defForce = new QPushButton("Define Force!");
	connect(butt_defForce, SIGNAL(released()), this, SLOT(setForceFlux()));
	QPushButton * butt_simStep = new QPushButton("Do 1 Timestep");
	connect(butt_simStep, SIGNAL(released()), this, SLOT(singleSimulationStep()));
	QPushButton * butt_startSim = new QPushButton("Start/Stop Simulation");
	connect(butt_startSim , SIGNAL(released()), this, SLOT(startSim()));

	QLabel * stepSliderLabel = new QLabel("Timestep Size [0,2]");
	QLabel * viscosityLabel = new QLabel("Viscosity [0,10]");
	forceAgeLabel = new QLabel("ForceAge (nr Iteratons): ");
	forceStrengthLabel = new QLabel("Force Strength (): ");

	stepSlider = new QSlider(Qt::Horizontal, this);
	stepSlider->setMinimum(0);
	stepSlider->setMaximum(1000);
	stepSlider->setTickPosition(QSlider::TicksAbove);
	stepSlider->setValue(10);
	connect(stepSlider,SIGNAL(sliderReleased()), this, SLOT(updateTimeStep()));


	viscositySlider = new QSlider(Qt::Horizontal, this);
	viscositySlider->setMinimum(0);
	viscositySlider->setMaximum(200);
	viscositySlider->setTickPosition(QSlider::TicksAbove);
	viscositySlider->setValue(0);
	connect(viscositySlider,SIGNAL(sliderReleased()), this, SLOT(updateViscosity()));


	forceAgeSlider = new QSlider(Qt::Horizontal, this);
	forceAgeSlider->setMinimum(0);
	forceAgeSlider->setMaximum(100);
	forceAgeSlider->setTickPosition(QSlider::TicksAbove);
	forceAgeSlider->setValue(5);
	connect(forceAgeSlider,SIGNAL(sliderReleased()), this, SLOT(forceAgeChanged()));
	forceAgeChanged();

	forceStrengthSlider = new QSlider(Qt::Horizontal, this);
	forceStrengthSlider->setMinimum(0);
	forceStrengthSlider->setMaximum(480);
	forceStrengthSlider->setTickPosition(QSlider::TicksAbove);
	forceStrengthSlider->setValue(80);
	connect(forceStrengthSlider,SIGNAL(sliderReleased()), this, SLOT(forceStrengthChanged()));
	forceStrengthChanged();


	viscosityAndTimestep = new QLabel("");
	updateViscTimeLabel();
	animationLabel = new QLabel("");

	vectorInput = new QLineEdit();

	QVBoxLayout * layout = new QVBoxLayout();

	layout->addWidget(butt2);
	layout->addWidget(butt);
	layout->addWidget(butt_defForce);
	layout->addWidget(butt_simStep);
	layout->addWidget(butt_startSim);
	layout->addWidget(stepSliderLabel);
	layout->addWidget(stepSlider);
	layout->addWidget(viscosityLabel);
	layout->addWidget(viscositySlider);
	layout->addWidget(forceAgeLabel);
	layout->addWidget(forceAgeSlider);
	layout->addWidget(forceStrengthLabel);
	layout->addWidget(forceStrengthSlider);
	layout->addWidget(viscosityAndTimestep);
	layout->addWidget(animationLabel);
	layout->addWidget(vectorInput);

	this->setLayout(layout);

	Model::getModel()->attach(this);
}

fluidControlWidget::~fluidControlWidget()
{
	if(mySimulation != NULL){
		delete mySimulation;
	}
}

void fluidControlWidget::initSimulation()
{
	this->mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
	Model::getModel()->setFluidSim(mySimulation);
	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();
	dirs.reserve(mesh.getBasicMesh().getFaces().size());
	for(int i = 0; i < mesh.getBasicMesh().getFaces().size(); i++){
		dirs.push_back(tuple3f());
	}

	dirs_cleared = true;
	updateTimeStep();
	updateViscosity();
}

float fluidControlWidget::getTimestep()
{
	return 2*(0.f +this->stepSlider->value())/this->stepSlider->maximum();
}

float fluidControlWidget::getViscosity()
{
	float temp = this->viscositySlider->value()*6 - 3*this->viscositySlider->maximum();
	temp/= this->viscositySlider->maximum();
	temp = pow(10,temp)- 0.00001;
	temp = temp < 0.000001?
		0:
		temp;
	return temp;
}

float fluidControlWidget::getForceStrength()
{
	float temp = this->forceStrengthSlider->value()*6 - this->forceStrengthSlider->maximum();
	temp/= this->forceStrengthSlider->maximum();
	temp = pow(10,temp)- 0.1;
	temp = temp < 0.00001?
		0:
	temp;
	return temp;
}


void fluidControlWidget::updateViscTimeLabel()
{
	stringstream ss;
	ss << "Timestep: " << getTimestep() <<", Viscosity: " << getViscosity();
	this->viscosityAndTimestep->setText(ss.str().c_str());
}


void fluidControlWidget::updateAnimationLabel(float time, float fps)
{
	stringstream ss;
	ss << "Time: " <<time <<", Fps: " << fps;
	this->animationLabel->setText(ss.str().c_str());
}


void fluidControlWidget::flux2vort2flux()
{

	if(mySimulation == NULL){
		initSimulation();
	}
	mySimulation->flux2Vorticity();

	updateViscosity();
	updateTimeStep();
	
	mySimulation->addDiffusion2Vorticity();


	mySimulation->vorticity2Flux();
	mySimulation->showFlux2Vel();


/*	mesh & m = * Model::getModel()->getMesh();
	float temp = 0;
	for(int i = 0; i < m.getVertices().size(); i++){
		temp += Operator::aVornoi(i,m);
	}
	
	cout << "Overall Area: " << temp << "\n";*/

}

void fluidControlWidget::getCollectedFlux()
{

	if(mySimulation == NULL){
		initSimulation();
	}
	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();

	std::vector<tuple3f> dirs;
	tuple3f dir(0.f,0.f,1.f);
	tuple3f n;

	vector<tuple3f> & constr_dirs = Model::getModel()->getInputCollector().getFaceDir();
	vector<int> & constr_fcs = Model::getModel()->getInputCollector().getFaces();

	for(int i = 0; i < mesh.getBasicMesh().getFaces().size(); i++){

		dirs.push_back(tuple3f());
	}

	for(int i = 0; i < constr_dirs.size(); i++){
		dirs[constr_fcs[i]] = constr_dirs[i];
	}

	mySimulation->setFlux(dirs);
	mySimulation->flux2Vorticity();
	
	mySimulation->showFlux2Vel();

	

	// the test.
	// the test fails if neighboring faces have "incompatible" fluxes. So nvm
	/*oneForm & f = mySimulation->getFlux();
	tuple3f n_ab, n_bc, n_ca;
	float test, test2;
	for(int j = 0; j < constr_dirs.size(); j++){
		int i = constr_fcs[j];
		tuple3f & a = verts[fcs[i].a];
		tuple3f & b = verts[fcs[i].b];
		tuple3f & c = verts[fcs[i].c];

		n = (b-a).cross(c-a);
		n.normalize();
		//normals
		n_ab = (b-a).cross(n);
		//n_ab.normalize(); unnormalized is correct: flux is times sidelength
		n_bc = (c-b).cross(n);
		//n_bc.normalize();
		n_ca = (a-c).cross(n);
		//n_ca.normalize();

		test = n.dot(dirs[i]);
		assert(test < 0.00001 && test > -0.00001);
		test = f.get(f2e[i].a,fcs[i].orientation(edges[f2e[i].a]));
		test2 = n_ab.dot(dirs[i]);
		test = test - test2;
		//test = n_ab.dot(dirs[i]);
		//test = test - f.get(f2e[i].a,fcs[i].orientation(edges[f2e[i].a]));
		assert(test < 0.00001 && test > -0.00001);
		test = n_bc.dot(dirs[i]);
		test = test - f.get(f2e[i].b,fcs[i].orientation(edges[f2e[i].b]));
		assert(test < 0.00001 && test > -0.00001);
		test = n_ca.dot(dirs[i]);
		test = test - f.get(f2e[i].c,fcs[i].orientation(edges[f2e[i].c]));
		assert(test < 0.00001 && test > -0.00001);
		test = test;
	}*/
}

void fluidControlWidget::update( void * src, Model::modelMsg msg )
{
	if(msg == Model::NEW_MESH_CREATED && mySimulation != NULL){
		delete mySimulation;
		mySimulation = NULL;
		dirs.clear();
	}
}

void fluidControlWidget::singleSimulationStep()
{
	if(mySimulation == NULL){
		initSimulation();
	}
	//this->mySimulation->pathTraceAndShow((0.f +this->stepSlider->value())/100);
	this->updateTimeStep();
	this->mySimulation->oneStep();
	updateAnimationLabel(mySimulation->getSimTime(), mySimulation->getFPS());
	mySimulation->showFlux2Vel();
}

void fluidControlWidget::updateTimeStep()
{
	if(mySimulation == NULL){
		initSimulation();
	}
	stepSize = getTimestep();
	updateViscTimeLabel();
	this->mySimulation->setStepSize(stepSize);
	this->mySimulation->pathTraceAndShow(stepSize);
}

void fluidControlWidget::setForceFlux()
{
	if(mySimulation == NULL){
		initSimulation();

	}

	vector<tuple3f> & constr_dirs = Model::getModel()->getInputCollector().getFaceDir();
	vector<int> & constr_fcs = Model::getModel()->getInputCollector().getFaces();

	float strength = getForceStrength();
	for(int i = 0; i < constr_dirs.size(); i++){
		dirs[constr_fcs[i]] = constr_dirs[i] * strength;
	}

	mySimulation->setForce(dirs);
}

void fluidControlWidget::updateViscosity()
{
	if(mySimulation == NULL){
		initSimulation();
	}

	updateViscTimeLabel();
	float viscy = getViscosity();
	mySimulation->setViscosity(viscy);
}


void fluidControlWidget::forceAgeChanged()
{
	this->maxForceAge = forceAgeSlider->value();
	stringstream ss;
	ss << "ForceAge (nr Iteratons): " << this->maxForceAge;
	this->forceAgeLabel->setText(ss.str().c_str());
}
	

void fluidControlWidget::doAnimation()
{

	if(mySimulation == NULL){
		initSimulation();
	}

	vector<tuple3f> & constr_dirs = Model::getModel()->getInputCollector().getFaceDir();
	vector<int> & constr_fcs = Model::getModel()->getInputCollector().getFaces();

	float strength = getForceStrength();
	if(constr_dirs.size() > 0){
		for(int i = 0; i < constr_dirs.size(); i++){
			dirs[constr_fcs[i]] = constr_dirs[i]*strength;
		}
		mySimulation->setForce(dirs);
		Model::getModel()->getInputCollector().clear();
		forceAge = 0;
		dirs_cleared = false;
	}


	mySimulation->oneStep();
	Model::getModel()->updateObserver(Model::DISPLAY_CHANGED);
	forceAge += 1;

	if(forceAge > maxForceAge && !dirs_cleared){
		for(int i = 0; i < dirs.size(); i++){
			dirs[i].set(0,0,0);
		}
		dirs_cleared = true;
		mySimulation->setForce(dirs);
	}

	updateAnimationLabel(mySimulation->getSimTime(), mySimulation->getFPS());
}

void fluidControlWidget::startSim()
{
	if(animationTimer->isActive()){
		animationTimer->stop();
	}
	else{
		animationTimer->start(40);
	}
}

void fluidControlWidget::forceStrengthChanged()
{
	stringstream ss;
	ss << "Force Strength (" << getForceStrength() << " ):";
	this->forceStrengthLabel->setText(ss.str().c_str());
}







