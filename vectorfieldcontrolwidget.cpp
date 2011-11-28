#include "vectorfieldcontrolwidget.h"
#include <Qt>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include "Model.h"
#include "mesh.h"
#include "meshOperation.h"
#include "VectorField.h"

#include "constraintCollector.h"
#include <QRadioButton>

#include <math.h>
#include "mystatusbar.h"

vectorFieldControlWidget::vectorFieldControlWidget(QWidget *parent)
	: QWidget(parent)
{

	weightStep = 100;
	srcFlowStep = 10;

	QPushButton *butt = new QPushButton("Generate VField!");
	connect(butt, SIGNAL(released()), this, SLOT(genAxisAllignedField()));

	QPushButton *butt2 = new QPushButton("Solve VField!");
	connect(butt2, SIGNAL(released()), this, SLOT(solveVField()));

	QRadioButton * rbutt = new QRadioButton("Select Sources", this);
	connect(rbutt, SIGNAL(toggled(bool)), this, SLOT(sourceSelection(bool)));
	QRadioButton * rbutt2 = new QRadioButton("Select Sinks", this);
	connect(rbutt2, SIGNAL(toggled(bool)), this, SLOT(sinkSelection(bool)));
	QRadioButton * rbutt3 = new QRadioButton("Select Guide Field", this);
	connect(rbutt3, SIGNAL(toggled(bool)), this, SLOT(fieldSelection(bool)));

	slider = new QSlider(Qt::Horizontal, this);
	slider->setMinimum(0);
	slider->setMaximum(10* weightStep);
	slider->setTickPosition(QSlider::TicksAbove);
	slider->setValue(weightStep);
	connect(slider, SIGNAL(sliderReleased()), this, SLOT(solveVField()));

	slider2 = new QSlider(Qt::Horizontal, this);
	slider2->setMinimum(0);
	slider2->setMaximum(50*srcFlowStep);
	slider2->setTickPosition(QSlider::TicksAbove);
	slider2->setValue(srcFlowStep);
	connect(slider2, SIGNAL(sliderReleased()), this, SLOT(solveVField()));

	slider3 = new QSlider(Qt::Horizontal, this);
	slider3->setMinimum(0);
	slider3->setMaximum(10*srcFlowStep);
	slider3->setTickPosition(QSlider::TicksAbove);
	slider3->setValue(0);
	connect(slider3, SIGNAL(sliderReleased()), this, SLOT(solveVField()));


	QLabel * sliderLabel1 = new QLabel("Guide Field Enforcement:", this);
	QLabel * sliderLabel2 = new QLabel("Source Flow:", this);
	QLabel * sliderLabel3 = new QLabel("Constraint Field length:", this);

	QVBoxLayout * layout = new QVBoxLayout();
//	layout->addWidget(cbox);
	layout->addWidget(rbutt);
	layout->addWidget(rbutt2);
	layout->addWidget(rbutt3);
	layout->addWidget(sliderLabel2);
	layout->addWidget(slider2);
	layout->addWidget(sliderLabel1);
	layout->addWidget(slider);
	layout->addWidget(sliderLabel3);
	layout->addWidget(slider3);
	layout->addWidget(butt);
	layout->addWidget(butt2);

	this->setLayout(layout);

//	this->solver = new VectorFieldSolver(Model::getModel()->getMesh(), 
//		*Model::getModel()->getMeshInfo()->getHalfedges(), //
//		*Model::getModel()->getMeshInfo()->getFace2Halfedges());

	this->solver = NULL;

	Model::getModel()->attach(this);

	this->mainWindow = NULL;
}

vectorFieldControlWidget::~vectorFieldControlWidget()
{
	delete solver;
}

void vectorFieldControlWidget::genAxisAllignedField()
{
	std::cout << "click";
	/* generate VField */
	/*mesh * m = Model::getModel()->getMesh();
	VectorField * field = new VectorField(m, tuple3f(0,0,1));
	//get halfEdge stuff...
	Model::getModel()->setVField(field);
	Model::getModel()->attach(this);*/
	
}

void vectorFieldControlWidget::solveVField()
{
	/*VectorFieldSolver solver(Model::getModel()->getMesh(), 
		*Model::getModel()->getMeshInfo()->getHalfedges(), 
		*Model::getModel()->getMeshInfo()->getFace2Halfedges());*/

	if(solver == NULL){
		this->initSolver();
	}

	vector<int> verts;
	vector<float> constr;

	fieldConstraintCollector & collector = Model::getModel()->getInputCollector();

	float srcFlow = slider2->value();
	srcFlow /= srcFlowStep;

	float weight = slider->value();
	weight = pow(10.f, weight/ weightStep) -1;
	weight = (weight > 0 ? weight: 0.f);

	float constraintLength = pow(10, (-1.f / srcFlowStep) * slider3->value());

	for(int i = 0; i < collector.sinkVert.size(); i++){
		verts.push_back(collector.sinkVert[i]);
		constr.push_back(-srcFlow);
	}
	for(int i = 0; i < collector.sourceVert.size(); i++){
		verts.push_back(collector.sourceVert[i]);
		constr.push_back(srcFlow);
	}

	solver->perturb(verts, constr);


	/*verts.push_back(0);
	verts.push_back(Model::getModel()->getMesh()->getVertices().size()/2);
	constr.push_back(1.f);
	constr.push_back(-1.f);*/

	if(Model::getModel()->getVField() == NULL){
		Model::getModel()->initVectorField();
	}
	solver->solve(verts, constr,
		Model::getModel()->getInputCollector().getEdges(),
		Model::getModel()->getInputCollector().getEdgeDirs(),
		weight,
		constraintLength,
		Model::getModel()->getVField());

	if(mainWindow != NULL){
		mainWindow->update();
	}
}

void vectorFieldControlWidget::sourceSelection( bool active )
{
	if(active){
		Model::getModel()->getInputCollector().setWhatToCollect(SOURCE_VERTS);
	}
}

void vectorFieldControlWidget::sinkSelection( bool active )
{
	if(active){
		Model::getModel()->getInputCollector().setWhatToCollect(SINK_VERTS);
	}
}

void vectorFieldControlWidget::fieldSelection( bool active )
{
	if(active){
		Model::getModel()->getInputCollector().setWhatToCollect(GUIDING_FIELD);
	}
}

void vectorFieldControlWidget::update( void * src, Model::modelMsg msg )
{
	if(msg == Model::NEW_MESH_CREATED){
		if(solver != NULL){
			delete solver;
		}
		solver = NULL;
	}
}

void vectorFieldControlWidget::setMainWindow( MainWindow * w)
{
	this->mainWindow = w;
}

void vectorFieldControlWidget::initSolver()
{
	myStatusBar bar(NULL);
	bar.open();
	bar.setBar(0,10);
	bar.updateBar(0);
	this->solver = new VectorFieldSolver(Model::getModel()->getMesh(), 
		*Model::getModel()->getMeshInfo()->getHalfedges(), 
		*Model::getModel()->getMeshInfo()->getFace2Halfedges(), &bar);
}
