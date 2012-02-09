#include "fluidcontrolwidget.h"
#include <QPushButton>
#include "QVBoxLayout"
#include "meshMetaInfo.h"
#include "dualMeshTools.h"
#include "fluidTools.h"
#include "oneForm.h"
#include <QLabel>

fluidControlWidget::fluidControlWidget(QWidget *parent)
	: QWidget(parent)
{

	mySimulation = NULL;

	QPushButton * butt = new QPushButton("Flux 2 Vorticity 2 Flux!");
	connect(butt, SIGNAL(released()), this, SLOT(flux2vort2flux()));
	QPushButton * butt2 = new QPushButton("Define Flux!");
	connect(butt2, SIGNAL(released()), this, SLOT(getCollectedFlux()));
	QPushButton * butt_defForce = new QPushButton("Define Force!");
	connect(butt_defForce, SIGNAL(released()), this, SLOT(setForceFlux()));
	QPushButton * butt3 = new QPushButton("New Fluid Sim");
	connect(butt3, SIGNAL(released()), this, SLOT(newFluidSim()));

	QLabel * stepSliderLabel = new QLabel("Timestep Size [0,2]");
	QLabel * viscosityLabel = new QLabel("Viscosity [0,10]");

	stepSlider = new QSlider(Qt::Horizontal, this);
	stepSlider->setMinimum(0);
	stepSlider->setMaximum(200);
	stepSlider->setTickPosition(QSlider::TicksAbove);
	stepSlider->setValue(10);
	connect(stepSlider,SIGNAL(sliderReleased()), this, SLOT(stepSizeChanged()));

	viscositySlider = new QSlider(Qt::Horizontal, this);
	viscositySlider->setMinimum(0);
	viscositySlider->setMaximum(200);
	viscositySlider->setTickPosition(QSlider::TicksAbove);
	viscositySlider->setValue(0);
	connect(viscositySlider,SIGNAL(sliderReleased()), this, SLOT(viscosityChanged()));

	QVBoxLayout * layout = new QVBoxLayout();

	layout->addWidget(butt2);
	layout->addWidget(butt);
	layout->addWidget(butt_defForce);
	layout->addWidget(butt3);
	layout->addWidget(stepSliderLabel);
	layout->addWidget(stepSlider);
	layout->addWidget(viscosityLabel);
	layout->addWidget(viscositySlider);

	this->setLayout(layout);

	Model::getModel()->attach(this);
}

fluidControlWidget::~fluidControlWidget()
{
	if(mySimulation != NULL){
		delete mySimulation;
	}
}

void fluidControlWidget::flux2vort2flux()
{

	if(mySimulation == NULL){
		mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
		Model::getModel()->setFluidSim(mySimulation);
	}
	mySimulation->flux2Vorticity();
	mySimulation->vorticity2Flux();
	//mySimulation->updateVelocities();
	mySimulation->showFlux2Vel();

}

void fluidControlWidget::getCollectedFlux()
{

	if(mySimulation == NULL){
		mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
		Model::getModel()->setFluidSim(mySimulation);
	}
	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();

/*	std::vector<tuple2i> & edges = * mesh.getHalfedges();
	std::vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	std::vector<tuple3i> & f2e = * mesh.getFace2Halfedges();	
	std::vector<tuple3i> & fcs = mesh.getBasicMesh().getFaces();*/

	std::vector<tuple3f> dirs;
	tuple3f dir(0.f,0.f,1.f);
	tuple3f n;

	vector<tuple3f> & constr_dirs = Model::getModel()->getInputCollector().getFaceDir();
	vector<int> & constr_fcs = Model::getModel()->getInputCollector().getFaces();

	for(int i = 0; i < mesh.getBasicMesh().getFaces().size(); i++){
/*		tuple3f & a = verts[fcs[i].a];
		dirs.push_back(tuple3f());
	}

	float tmp;
	for(int i = 0; i < constrFcs.size(); i++){
		tuple3f & a = verts[fcs[constrFcs[i]].a];
		tuple3f & b = verts[fcs[constrFcs[i]].b];
		tuple3f & c = verts[fcs[constrFcs[i]].c];

		n = (b-a).cross(c-a);
		n.normalize();*/
		
		dirs.push_back(tuple3f());
		assert(tmp < 0.0001 && tmp > -0.0001);

		dirs[constrFcs[i]] = constrDir[i];
	}

	for(int i = 0; i < constr_dirs.size(); i++){
		dirs[constr_fcs[i]] = constr_dirs[i];
	}

	mySimulation->setFlux(dirs);
	mySimulation->flux2Vorticity();
//	mySimulation->vorticity2Flux();
	
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
	}
}

void fluidControlWidget::newFluidSim()
{
	if(mySimulation == NULL){
		this->mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
		Model::getModel()->setFluidSim(mySimulation);
	}
	//this->mySimulation->pathTraceAndShow((0.f +this->stepSlider->value())/100);
	this->mySimulation->oneStep((0.f +this->stepSlider->value())/100);
	mySimulation->showFlux2Vel();
}

void fluidControlWidget::stepSizeChanged()
{
	stepSize = (0.f +this->stepSlider->value())/100;

	if(mySimulation == NULL){
		this->mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
		Model::getModel()->setFluidSim(mySimulation);
	}

	this->mySimulation->setStepSize(stepSize);
	this->mySimulation->pathTraceAndShow(stepSize);
}

void fluidControlWidget::setForceFlux()
{
	if(mySimulation == NULL){
		this->mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
		Model::getModel()->setFluidSim(mySimulation);
	}

	vector<tuple3f> & constr_dirs = Model::getModel()->getInputCollector().getFaceDir();
	vector<int> & constr_fcs = Model::getModel()->getInputCollector().getFaces();
	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();

	vector<tuple3f> dirs;
	dirs.reserve(mesh.getBasicMesh().getFaces().size());
	for(int i = 0; i < mesh.getBasicMesh().getFaces().size(); i++){
		dirs.push_back(tuple3f());
	}

	for(int i = 0; i < constr_dirs.size(); i++){
		dirs[constr_fcs[i]] = constr_dirs[i];
	}

	mySimulation->setForce(dirs);
}

void fluidControlWidget::viscosityChanged()
{
	if(mySimulation == NULL){
		this->mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
		Model::getModel()->setFluidSim(mySimulation);
	}
	float viscy = (0.f +this->viscositySlider->value())/20;
	mySimulation->setViscosity(viscy);
}
