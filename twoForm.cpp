#include "twoForm.h"


twoForm::twoForm( meshMetaInfo & mesh )
{
	this->m = &mesh;
	this->form.reserve(mesh.getBasicMesh().getFaces().size());
}

twoForm::~twoForm(void)
{
}
