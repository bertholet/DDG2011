#include "fluidcontrolwidget.h"
#include <QPushButton>
#include "QVBoxLayout"
#include "meshMetaInfo.h"
#include "dualMeshTools.h"
#include "fluidTools.h"
#include "oneForm.h"

fluidControlWidget::fluidControlWidget(QWidget *parent)
	: QWidget(parent)
{

	mySimulation = NULL;

	QPushButton * butt = new QPushButton("Circumcenters!");
	connect(butt, SIGNAL(released()), this, SLOT(circumcenters()));
	QPushButton * butt2 = new QPushButton("Flux2VectorExample!");
	connect(butt2, SIGNAL(released()), this, SLOT(flux2Vel()));
	QPushButton * butt3 = new QPushButton("New Fluid Sim");
	connect(butt3, SIGNAL(released()), this, SLOT(newFluidSim()));


	stepSlider = new QSlider(Qt::Horizontal, this);
	stepSlider->setMinimum(0);
	stepSlider->setMaximum(200);
	stepSlider->setTickPosition(QSlider::TicksAbove);
	stepSlider->setValue(10);
	connect(stepSlider,SIGNAL(sliderReleased()), this, SLOT(stepSizeChanged()));

	QVBoxLayout * layout = new QVBoxLayout();

	layout->addWidget(butt);
	layout->addWidget(butt2);
	layout->addWidget(butt3);
	layout->addWidget(stepSlider);

	this->setLayout(layout);

	Model::getModel()->attach(this);
}

fluidControlWidget::~fluidControlWidget()
{
	if(mySimulation != NULL){
		delete mySimulation;
	}
}

void fluidControlWidget::circumcenters()
{

	if(mySimulation == NULL){
		mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
	}
	mySimulation->showDualPositions();

}

void fluidControlWidget::flux2Vel()
{

	if(mySimulation == NULL){
		mySimulation = new fluidSimulation(Model::getModel()->getMeshInfo());
	}
	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();

	std::vector<tuple2i> & edges = * mesh.getHalfedges();
	std::vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	std::vector<tuple3i> & f2e = * mesh.getFace2Halfedges();	
	std::vector<tuple3i> & fcs = mesh.getBasicMesh().getFaces();

	std::vector<tuple3f> dirs;
	tuple3f dir(0.f,0.f,1.f);
	tuple3f n;

	std::vector<tuple3f> & constrDir = Model::getModel()->getInputCollector().getFaceDir();
	std::vector<int> & constrFcs = Model::getModel()->getInputCollector().getFaces();

	for(int i = 0; i < mesh.getBasicMesh().getFaces().size(); i++){

		dirs.push_back(tuple3f());
	}

	float tmp;
	for(int i = 0; i < constrFcs.size(); i++){
		tuple3f & a = verts[fcs[constrFcs[i]].a];
		tuple3f & b = verts[fcs[constrFcs[i]].b];
		tuple3f & c = verts[fcs[constrFcs[i]].c];

		n = (b-a).cross(c-a);
		n.normalize();
		
		tmp = n.dot(dirs[i]);
		assert(tmp < 0.0001 && tmp > -0.0001);

		dirs[constrFcs[i]] = constrDir[i];
	}


	mySimulation->setFlux(dirs);

	
/*	std::vector<double> & sth = f.getVals();
	tuple3f n;
	for(int i = 0; i < fcs.size(); i++){
		tuple3f & a = verts[fcs[i].a];
		tuple3f & b = verts[fcs[i].b];
		tuple3f & c = verts[fcs[i].c];
		tuple3i & f_edgs = f2e[i];

		n = (b-a).cross(c-a);
		n.normalize();


		sth[f_edgs.a] = ((verts[edges[f_edgs.a].b]-verts[edges[f_edgs.a].a])).cross(n).dot(dir);
		sth[f_edgs.b] = (verts[edges[f_edgs.b].b]-verts[edges[f_edgs.b].a]).cross(n).dot(dir);
		sth[f_edgs.c] = (verts[edges[f_edgs.c].b]-verts[edges[f_edgs.c].a]).cross(n).dot(dir);
	}*/
	
	
	//std::vector<tuple3f> velocities;
	//fluidTools::flux2Velocity(f,velocities, mesh);
	//mySimulation->setFlux(f);
	mySimulation->showFlux2Vel();

	

	// the test.
/*	oneForm & f = mySimulation->getFlux();
	tuple3f n_ab, n_bc, n_ca;
	float test, test2;
	for(int i = 0; i < dirs.size(); i++){
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
	}
	//this->mySimulation->pathTraceAndShow((0.f +this->stepSlider->value())/100);
	this->mySimulation->oneStep((0.f +this->stepSlider->value())/100);
}

void fluidControlWidget::stepSizeChanged()
{
	stepSize = (0.f +this->stepSlider->value())/100;
	newFluidSim();
}
