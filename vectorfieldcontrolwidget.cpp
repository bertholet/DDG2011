#include "vectorfieldcontrolwidget.h"
#include <Qt>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <iostream>
#include "Model.h"
#include "mesh.h"
#include "meshOperation.h"
#include "VectorField.h"

#include "constraintCollector.h"
#include <QRadioButton>

#include <math.h>
#include "mystatusbar.h"
#include "ObjFileWriter.h"

vectorFieldControlWidget::vectorFieldControlWidget(QWidget *parent)
	: QWidget(parent)
{

	weightStep = 250;
	weightMax = 500;
	srcFlowStep = 20;
	lengthStep = 100;

	QPushButton *butt = new QPushButton("Generate VField!");
	connect(butt, SIGNAL(released()), this, SLOT(genAxisAllignedField()));

	QPushButton *butt2 = new QPushButton("Solve VField!");
	connect(butt2, SIGNAL(released()), this, SLOT(solveVField()));

	QPushButton *butt3 =new QPushButton("Store VField!");
	connect(butt3, SIGNAL(released()), this, SLOT(storeField()));


	QRadioButton * rbutt = new QRadioButton("Select Sources", this);
	connect(rbutt, SIGNAL(toggled(bool)), this, SLOT(sourceSelection(bool)));
	QRadioButton * rbutt2 = new QRadioButton("Select Sinks", this);
	connect(rbutt2, SIGNAL(toggled(bool)), this, SLOT(sinkSelection(bool)));
	QRadioButton * rbutt3 = new QRadioButton("Select Guide Field", this);
	connect(rbutt3, SIGNAL(toggled(bool)), this, SLOT(fieldSelection(bool)));

	gfWeihgtSlider = new QSlider(Qt::Horizontal, this);
	gfWeihgtSlider->setMinimum(0);
	gfWeihgtSlider->setMaximum(weightMax);
	gfWeihgtSlider->setTickPosition(QSlider::TicksAbove);
	gfWeihgtSlider->setValue(weightStep);
	connect(gfWeihgtSlider, SIGNAL(sliderReleased()), this, SLOT(solveVField()));

	flowSlider = new QSlider(Qt::Horizontal, this);
	flowSlider->setMinimum(0);
	flowSlider->setMaximum(10*srcFlowStep);
	flowSlider->setTickPosition(QSlider::TicksAbove);
	flowSlider->setValue(srcFlowStep);
	connect(flowSlider, SIGNAL(sliderReleased()), this, SLOT(solveVField()));

	gfLengthSlider = new QSlider(Qt::Horizontal, this);
	gfLengthSlider->setMinimum(0);
	gfLengthSlider->setMaximum(5*lengthStep);
	gfLengthSlider->setTickPosition(QSlider::TicksAbove);
	gfLengthSlider->setValue(lengthStep);
	connect(gfLengthSlider, SIGNAL(sliderReleased()), this, SLOT(solveVField()));


	QLabel * sliderLabel1 = new QLabel("Guide Field Enforcement:", this);
	QLabel * sliderLabel2 = new QLabel("Source Flow:", this);
	QLabel * sliderLabel3 = new QLabel("Constraint Field length:", this);

	cBoxDirectional = new QCheckBox("Directional Constraint", this);

	QCheckBox * cBoxBorderMatrix = new QCheckBox("Border Matrix", this);
	cBoxBorderMatrix->setChecked(true);
	useBorderMat = true;
	connect(cBoxBorderMatrix, SIGNAL(stateChanged(int)), this, SLOT(useBorderMatrix(int)));

//	QCheckBox * cBoxArrow = new QCheckBox("Show Arrows", this);
//	cBoxArrow->setChecked(false);
//	connect(cBoxArrow, SIGNAL(stateChanged(int)), this, SLOT(showArrows(int)));

	QVBoxLayout * layout = new QVBoxLayout();
//	layout->addWidget(cbox);
	layout->addWidget(rbutt);
	layout->addWidget(rbutt2);
	layout->addWidget(rbutt3);
	layout->addWidget(sliderLabel2);
	layout->addWidget(flowSlider);
	layout->addWidget(sliderLabel1);
	layout->addWidget(gfWeihgtSlider);
	layout->addWidget(sliderLabel3);
	layout->addWidget(gfLengthSlider);
	layout->addWidget(butt);
	layout->addWidget(butt2);
	layout->addWidget(butt3);
	layout->addWidget(cBoxDirectional);
	layout->addWidget(cBoxBorderMatrix);
	//layout->addWidget(cBoxArrow);
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

	float srcFlow = flowSlider->value();
	srcFlow /= srcFlowStep;
	srcFlow = pow(10, srcFlow-1);

	float weight = gfWeihgtSlider->value();
	weight = 8*weight/weightStep;
	weight = pow(10, weight-8) - pow(10,-8.f);
	weight = (weight > 0 ? weight: 0.f);

	float constraintLength = pow(10, -1 + (0.f + gfLengthSlider->value())/lengthStep);

	for(int i = 0; i < collector.sinkVert.size(); i++){
		verts.push_back(collector.sinkVert[i]);
		constr.push_back(srcFlow);
	}
	for(int i = 0; i < collector.sourceVert.size(); i++){
		verts.push_back(collector.sourceVert[i]);
		constr.push_back(-srcFlow);
	}

	solver->perturb(verts, constr);


	/*verts.push_back(0);
	verts.push_back(Model::getModel()->getMesh()->getVertices().size()/2);
	constr.push_back(1.f);
	constr.push_back(-1.f);*/

	if(Model::getModel()->getVField() == NULL){
		Model::getModel()->initVectorField();
	}

	if(! cBoxDirectional->isChecked()){
		solver->solve(verts, constr,
			Model::getModel()->getInputCollector().getEdges(),
			Model::getModel()->getInputCollector().getEdgeDirs(),
			weight * (Model::getModel()->getMeshInfo()->getHalfedges()->size()),
			constraintLength,
			Model::getModel()->getVField(),
			useBorderMat);
	}
	else{
	/*	solver->solveDirectional(verts, constr,
			Model::getModel()->getInputCollector().getFaces(),
			Model::getModel()->getInputCollector().getFaceDir(),
			weight * (Model::getModel()->getMeshInfo()->getHalfedges()->size()),
			constraintLength,
			Model::getModel()->getVField());*/
		solver->solveLengthEstimated(verts, constr,
			Model::getModel()->getInputCollector().getFaces(),
			Model::getModel()->getInputCollector().getFaceDir(),
			weight * (Model::getModel()->getMeshInfo()->getHalfedges()->size()),
			Model::getModel()->getVField(),
			useBorderMat);
	}

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

void vectorFieldControlWidget::storeField()
{

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Select Obj. File"), "/home/", tr("Obj Files (*.obj)"));

	if(!fileName.endsWith("obj")){
		QMessageBox msgBox;
		msgBox.setText("Not an Obj file");
		return;
	}

	ObjFileWriter writer;
	writer.writeObjFile(fileName.toAscii(),*(Model::getModel()->getMesh()),*(Model::getModel()->getVField()));
}

void vectorFieldControlWidget::useBorderMatrix( int val )
{
	useBorderMat = (val == 2);
}

