#include "fluidcontrolwidget.h"
#include <QPushButton>
#include "QVBoxLayout"
#include "meshMetaInfo.h"
#include "dualMeshTools.h"

fluidControlWidget::fluidControlWidget(QWidget *parent)
	: QWidget(parent)
{
	QPushButton * butt = new QPushButton("Circumcenters!");
	connect(butt, SIGNAL(released()), this, SLOT(circumcenters()));
	QPushButton * butt2 = new QPushButton("Flux2VectorExample!");
	connect(butt2, SIGNAL(released()), this, SLOT(flux2Vel()));

	QVBoxLayout * layout = new QVBoxLayout();

	layout->addWidget(butt);
	layout->addWidget(butt2);

	this->setLayout(layout);

	Model::getModel()->attach(this);
}

fluidControlWidget::~fluidControlWidget()
{

}

void fluidControlWidget::circumcenters()
{

	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();
	dualMeshTools::getDualVertices(mesh, dualVertices);
	int a = dualVertices.size();
	Model::getModel()->setPointCloud(&dualVertices);

}

void fluidControlWidget::flux2Vel()
{

}

void fluidControlWidget::update( void * src, Model::modelMsg msg )
{

}
