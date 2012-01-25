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
	std::vector<tuple3i> & fcs = mesh.getBasicMesh().getFaces();
	
	std::vector<double> & sth = f.getVals();
	tuple3f n;
	for(int i = 0; i < fcs.size(); i++){
		tuple3f & a = verts[fcs[i].a];
		tuple3f & b = verts[fcs[i].b];
		tuple3f & c = verts[fcs[i].c];
		tuple3i & f_edgs = f2e[i];

		n = (b-a).cross(c-a);
		n.normalize();


		sth[f_edgs.a] = ((verts[edges[f_edgs.a].b]-verts[edges[f_edgs.a].a])).cross(n).dot(tuple3f(0,0,1));
		sth[f_edgs.b] = (verts[edges[f_edgs.b].b]-verts[edges[f_edgs.b].a]).cross(n).dot(tuple3f(0,0,1));
		sth[f_edgs.c] = (verts[edges[f_edgs.c].b]-verts[edges[f_edgs.c].a]).cross(n).dot(tuple3f(0,0,1));
	}
	
	
	std::vector<tuple3f> velocities;
	fluidTools::flux2Velocity(f,velocities, mesh);
	
	//display hack
	VectorField * fld = new VectorField(&mesh.getBasicMesh());
	for(int i = 0; i < mesh.getBasicMesh().getFaces().size(); i++){

		fld->setOneForm(i,velocities[i]);
	}
	Model::getModel()->setVField(fld);


/*	// the test.

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
		test = test - f.get(f2e[i].a,fcs[i].orientation(edges[f2e[i].a]));
		test = n_bc.dot(velocities[i]);
		test = test - f.get(f2e[i].b,fcs[i].orientation(edges[f2e[i].b]));
		test = n_ca.dot(velocities[i]);
		test = test - f.get(f2e[i].c,fcs[i].orientation(edges[f2e[i].c]));
		test = test;
	}*/
}

void fluidControlWidget::update( void * src, Model::modelMsg msg )
{

}
