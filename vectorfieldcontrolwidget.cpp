#include "vectorfieldcontrolwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include "Model.h"
#include "mesh.h"
#include "meshOperation.h"
#include "VectorField.h"

vectorFieldControlWidget::vectorFieldControlWidget(QWidget *parent)
	: QWidget(parent)
{
	QPushButton *butt = new QPushButton("Generate VField!");
	connect(butt, SIGNAL(released()), this, SLOT(genAxisAllignedField()));

//	QCheckBox * cbox = new QCheckBox("Draw strokes",this);

	QVBoxLayout * layout = new QVBoxLayout();
//	layout->addWidget(cbox);
	layout->addWidget(butt);

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
