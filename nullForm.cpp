#include "nullForm.h"


nullForm::nullForm( meshMetaInfo & mesh )
{
	this->m = &mesh;
	this->form.reserve(mesh.getHalfedges()->size());

	int sz = mesh.getBasicMesh().getVertices().size();
	for(int i = 0; i < sz; i++){
		this->form.push_back(0);
	}
}

nullForm::~nullForm(void)
{
}

int nullForm::size()
{
	return form.size();
}

std::vector<double> & nullForm::getVals()
{
	return form;
}

double nullForm::get( int i, int orientation )
{
	return form[i]*orientation;
}
