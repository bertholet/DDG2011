#include "oneForm.h"


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

double oneForm::get( int i )
{
	return form[i];
}



