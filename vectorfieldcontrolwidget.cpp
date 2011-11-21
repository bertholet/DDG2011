#include "vectorfieldcontrolwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include "Model.h"
#include "mesh.h"
#include "meshOperation.h"
#include "VectorField.h"
#include "VectorFieldSolver.h"
#include "constraintCollector.h"
#include <QRadioButton>

vectorFieldControlWidget::vectorFieldControlWidget(QWidget *parent)
	: QWidget(parent)
{
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

	QVBoxLayout * layout = new QVBoxLayout();
//	layout->addWidget(cbox);
	layout->addWidget(rbutt);
	layout->addWidget(rbutt2);
	layout->addWidget(rbutt3);
	layout->addWidget(butt);
	layout->addWidget(butt2);

	this->setLayout(layout);
}

vectorFieldControlWidget::~vectorFieldControlWidget()
{

}

void vectorFieldControlWidget::genAxisAllignedField()
{
	std::cout << "click";
	/* generate VField */
	mesh * m = Model::getModel()->getMesh();
	VectorField * field = new VectorField(m, tuple3f(0,0,1));
	//get halfEdge stuff...
	Model::getModel()->setVField(field);
	
}

void vectorFieldControlWidget::solveVField()
{
	VectorFieldSolver solver(Model::getModel()->getMesh(), 
		*Model::getModel()->getMeshInfo()->getHalfedges(), 
		*Model::getModel()->getMeshInfo()->getFace2Halfedges());

	vector<int> verts;
	vector<float> constr;

	fieldConstraintCollector & collector = Model::getModel()->getInputCollector();

	for(int i = 0; i < collector.sinkVert.size(); i++){
		verts.push_back(collector.sinkVert[i]);
		constr.push_back(-1.f);
	}
	for(int i = 0; i < collector.sourceVert.size(); i++){
		verts.push_back(collector.sourceVert[i]);
		constr.push_back(1.f);
	}

	solver.perturb(verts, constr);


	/*verts.push_back(0);
	verts.push_back(Model::getModel()->getMesh()->getVertices().size()/2);
	constr.push_back(1.f);
	constr.push_back(-1.f);*/

	if(Model::getModel()->getVField() == NULL){
		Model::getModel()->initVectorField();
	}
	solver.solve(verts, constr,
		Model::getModel()->getInputCollector().getEdges(),
		Model::getModel()->getInputCollector().getEdgeDirs(),
		Model::getModel()->getVField());
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
