#include "vectorfieldcontrolwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include "Model.h"
#include "mesh.h"
#include "meshOperation.h"
#include "VectorField.h"
#include "VectorFieldSolver.h"

vectorFieldControlWidget::vectorFieldControlWidget(QWidget *parent)
	: QWidget(parent)
{
	QPushButton *butt = new QPushButton("Generate VField!");
	connect(butt, SIGNAL(released()), this, SLOT(genAxisAllignedField()));

	QPushButton *butt2 = new QPushButton("Solve VField!");
	connect(butt2, SIGNAL(released()), this, SLOT(solveVField()));

//	QCheckBox * cbox = new QCheckBox("Draw strokes",this);

	QVBoxLayout * layout = new QVBoxLayout();
//	layout->addWidget(cbox);
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
	VectorField * field = new VectorField(m);
	//get halfEdge stuff...
	Model::getModel()->setVField(field);
	
}

void vectorFieldControlWidget::solveVField()
{
	VectorFieldSolver solver(Model::getModel()->getMesh(), 
		*Model::getModel()->getMeshInfo()->getHalfedges(), 
		*Model::getModel()->getMeshInfo()->getFace2Halfedges());

	vector<int> verts;
	verts.push_back(0);
	verts.push_back(Model::getModel()->getMesh()->getVertices().size()/2);
	vector<float> constr;
	constr.push_back(1.f);
	constr.push_back(-1.f);
	solver.solve(verts, constr,Model::getModel()->getVField());
}
