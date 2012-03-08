#include "oneForm.h"
#include <assert.h>


oneForm::oneForm( meshMetaInfo & mesh )
{
	this->m = &mesh;
	this->form.reserve(mesh.getHalfedges()->size());

	int sz = mesh.getHalfedges()->size();
	for(int i = 0; i < sz; i++){
		this->form.push_back(0);
	}
	
}

oneForm::~oneForm(void)
{
}

int oneForm::size()
{
	return form.size();
}

std::vector<double> & oneForm::getVals()
{
	return form;
}

double oneForm::get( int i , int orientation)
{
	assert(orientation == -1 || orientation == 1);
	return form[i]* orientation;
}

void oneForm::set( int edge, double val, int orientation )
{
	assert(orientation == 1 || orientation == -1);
	form[edge] = val*orientation;
}

void oneForm::setZero()
{
	for(int i = 0; i < size(); i++){
		form[i] =0;
	}
}

void oneForm::initToConstFlux( tuple3f& dir )
{
	vector<tuple2i> & he = * m->getHalfedges();
	vector<tuple3f> & verts = m->getBasicMesh().getVertices();
	for(int i = 0; i < he.size(); i++){
		this->set(i,(verts[he[i].b] -verts[he[i].a]).dot(dir),1);
	}

}

void oneForm::add( oneForm & other )
{
	assert(other.m == this->m);
	for(int i = 0; i < form.size(); i++){
		form[i]+= other.form[i];
	}
}



