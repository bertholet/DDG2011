#pragma once
#include <string.h>
#include "VectorField.h"
#include "mesh.h"

class ObjFileWriter
{
public:
	ObjFileWriter(void);
	~ObjFileWriter(void);

	void writeObjFile(const char *file, mesh & m, VectorField & vf);
};
