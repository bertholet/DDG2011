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
#include <stdlib.h>
#include <assert.h>

#include "pardisoMatrix.h"
#include "DDGMatrices.h"
#include "pardiso.h"
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


	QPushButton * debug = new QPushButton("Debug!");
	connect(debug , SIGNAL(released()), this, SLOT(debugSome()));

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
	vectorInput->setText("");
	//vectorInput->setInputMask("#09.00 #09.00 #09.00");
	connect(vectorInput,SIGNAL(textChanged( const QString& )), this, SLOT(borderDirInput(const QString & )));

	QVBoxLayout * layout = new QVBoxLayout();

	layout->addWidget(butt2);
	layout->addWidget(butt);
	layout->addWidget(butt_defForce);
	layout->addWidget(butt_simStep);
	layout->addWidget(butt_startSim);

layout->addWidget(debug);


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


	this->selectedBorder = 0;
	for(int i = 0; i < Model::getModel()->getMeshInfo()->getBorder().size(); i++){
		borderConstrDirs.push_back(tuple3f());
	}

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

	
}

void fluidControlWidget::update( void * src, Model::modelMsg msg )
{
	if(msg == Model::NEW_MESH_CREATED && mySimulation != NULL){
		delete mySimulation;
		mySimulation = NULL;
		dirs.clear();
		borderConstrDirs.clear();
		this->selectedBorder = 0;

	}

	this->selectedBorder = 0;
	borderConstrDirs.clear();
	for(int i = 0; i < Model::getModel()->getMeshInfo()->getBorder().size(); i++){
		borderConstrDirs.push_back(tuple3f());
	}
}

void fluidControlWidget::update( void * src, borderMarkupMap * msg )
{
	this->selectedBorder = msg->markedBorder;
	stringstream ss;

	ss << borderConstrDirs[selectedBorder].x << " ";
	ss << borderConstrDirs[selectedBorder].y << " ";
	ss << borderConstrDirs[selectedBorder].z;

	//	this->vectorInput->setText("0 0 0");
	this->vectorInput->setText(ss.str().c_str());

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

void fluidControlWidget::borderDirInput( const QString & text )
{

	QByteArray ba = text.toLatin1();
	std::string buff = ba.data();

	float val1 =0;
	float val2=0;
	float val3=0;

	sscanf_s(buff.c_str(), "%f %f %f", &val1, &val2, &val3);

	if(borderConstrDirs.size() > selectedBorder){
		borderConstrDirs[selectedBorder].set(val1,val2,val3);
	}

}

void fluidControlWidget::debugSome()
{
	meshMetaInfo * mesh = Model::getModel()->getMeshInfo();
	pardisoMatrix star0 = DDGMatrices::star0(*mesh);
	star0.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/star0.m");

	pardisoMatrix star1 = DDGMatrices::star1(*mesh);
	star1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/star1.m");

	pardisoMatrix star2 = DDGMatrices::star2(*mesh);
	star2.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/star2.m");

	pardisoMatrix d0 = DDGMatrices::d0(*mesh);
	d0.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/d0.m");

	pardisoMatrix d1 = DDGMatrices::d1(*mesh);
	d1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/d1.m");

	pardisoMatrix delta1 = DDGMatrices::delta1(*mesh);
	d1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/delta1.m");

	pardisoMatrix delta2 = DDGMatrices::delta2(*mesh);
	d1.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/delta2.m");

	pardisoMatrix Lflux = delta2*d1 + d0*delta1;
	//Lflux = (DDGMatrices::id1(*mesh)%Lflux) * Lflux;

//	Lflux.setLineToID(3);
//	Lflux.setLineToID(5);

//	Lflux.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/Lflux.m");
	//set matrix to id on border
	vector<vector<int>> & brdr = mesh->getBorder();
	int sz;
	tuple2i edge;
	int edgeId;
	oneForm fluxConstraint(*mesh);

	vector<double> & fluxConstr = fluxConstraint.getVals();
	vector<tuple3f> & verts = mesh->getBasicMesh().getVertices();
	vector<double> buff = fluxConstr;
	vector<tuple2i> & edgs = * mesh->getHalfedges();
	vector<int> temp_indx;
	vector<double> temp_vals;;

	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
		for(int j = 0; j < sz;j++){
			edgeId =mesh->getHalfedgeId(brdr[i][j%sz], brdr[i][(j+1)%sz],&edge);
			assert(edgeId >=0);
			assert((edgs[edgeId].a == brdr[i][j%sz] && edgs[edgeId].b == brdr[i][(j+1)%sz] )||
					(edgs[edgeId].b == brdr[i][j%sz] && edgs[edgeId].a == brdr[i][(j+1)%sz]));
			//Lflux.setLineToID(edgeId);
			// i have to make sure there is no rot on the border and no divergence!!!!!!!!

			temp_indx.clear();
			temp_indx.push_back(edgeId);
			temp_vals.clear();
			temp_vals.push_back(1);
			
			Lflux.addLine(temp_indx, temp_vals);
			buff.push_back( borderConstrDirs[i].dot(verts[edge.b] -verts[edge.a]));

			//Lflux.add(edgeId,edgeId,1);
			//fluxConstr[edgeId] = borderConstrDirs[i].dot(verts[edge.b] -verts[edge.a]);

		}
	}


	Lflux.saveMatrix("C:/Users/bertholet/Dropbox/To Delete/debugSome/Lflux.m");
	Lflux.saveVector(fluxConstr, "fluxConstraint", "C:/Users/bertholet/Dropbox/To Delete/debugSome/LfluxConstr.m");

	Lflux = (DDGMatrices::id1(*mesh)%Lflux) * Lflux;
	(DDGMatrices::id1(*mesh)%Lflux).mult(buff, fluxConstr);

	/*pardisoMatrix d0delta1 = d0*delta1;
	d0delta1.mult(fluxConstr,buff);

	//////////////////////////////////////////////////////////////////////////
	// inefficient way to make fluxConstr = what it has to be on the border plus something
	// on corners etc
	fluxConstr = buff;
	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
		for(int j = 0; j < sz;j++){
			edgeId =mesh->getHalfedgeId(brdr[i][j%sz], brdr[i][(j+1)%sz],&edge);
			assert(edgeId >=0);
			assert((edgs[edgeId].a == brdr[i][j%sz] && edgs[edgeId].b == brdr[i][(j+1)%sz] )||
				(edgs[edgeId].b == brdr[i][j%sz] && edgs[edgeId].a == brdr[i][(j+1)%sz]));
			Lflux.setLineToID(edgeId);

			fluxConstr[edgeId] = borderConstrDirs[i].dot(verts[edge.b] -verts[edge.a]);

		}
	}*/

	pardisoSolver solver(pardisoSolver::MT_ANY, pardisoSolver::SOLVER_DIRECT,3);
	solver.setMatrix(Lflux,1);
	solver.setStoreResultInB(true);
	solver.solve(& (buff[0]), & (fluxConstr[0]));

	if(mySimulation ==  NULL){
		initSimulation();
	}
	mySimulation->setFlux(fluxConstraint);
	mySimulation->showFlux2Vel();

	/*Lflux.mult(fluxConstr,buff);

	float temp;
	for(int i = 0; i < brdr.size(); i++){
		sz =brdr[i].size();
		for(int j = 0; j < sz;j++){
			edgeId =mesh->getHalfedgeId(brdr[i][j%sz], brdr[i][(j+1)%sz],&edge);

			temp = borderConstrDirs[i].dot(verts[edge.b] -verts[edge.a]);
			assert(temp -buff[edgeId] < 0.0001 && temp -buff[edgeId] > -0.0001);

			buff[edgeId] = 0;

		}
	}

	for(int i = 0; i < buff.size(); i++){
			assert(buff[edgeId] < 0.0001 && buff[edgeId] > -0.0001);
	}*/

}







