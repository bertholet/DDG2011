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
	Model::getModel()->setPointCloud(&dualVertices);

}

void fluidControlWidget::flux2Vel()
{
	meshMetaInfo & mesh = * Model::getModel()->getMeshInfo();
	oneForm f(mesh);
	std::vector<tuple2i> & edges = * mesh.getHalfedges();
	std::vector<tuple3f> & verts = mesh.getBasicMesh().getVertices();
	std::vector<tuple3i> & f2e = * mesh.getFace2Halfedges();	
	
	std::vector<double> & sth = f.getVals();
	for(int i = 0; i < sth.size(); i++){
		sth[i] = (verts[edges[i].b]-verts[edges[i].a]).dot(tuple3f(1,1,1));
	}
	
	
	std::vector<tuple3f> velocities;
	fluidTools::flux2Velocity(f,velocities, mesh);
	int a = velocities.size();


	// the test.
	std::vector<tuple3i> & fcs = mesh.getBasicMesh().getFaces();
	tuple3f n;
	tuple3f n_ab, n_bc, n_ca;
	float test;
	for(int i = 0; i < velocities.size(); i++){
		tuple3f & a = verts[fcs[i].a];
		tuple3f & b = verts[fcs[i].b];
		tuple3f & c = verts[fcs[i].c];

		n = (b-a).cross(c-a);
		n.normalize();
		//normals
		n_ab = (b-a).cross(n);
		n_bc = (c-b).cross(n);
		n_ca = (a-c).cross(n);

		test = n.dot(velocities[i]);
		test = n_ab.dot(velocities[i]);
		test = test - sth[f2e[i].a];
		test = n_bc.dot(velocities[i]);
		test = test - sth[f2e[i].b];
		test = n_ca.dot(velocities[i]);
		test = test - sth[f2e[i].c];
		test = test;
	}
}

void fluidControlWidget::update( void * src, Model::modelMsg msg )
{

}
