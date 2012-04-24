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



