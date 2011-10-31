#include "VectorField.h"

VectorField::VectorField( mesh * aMesh )
{
	aMesh->attach(this);
}

VectorField::~VectorField(void)
{

}

void VectorField::update( void * src, int msg )
{

}
