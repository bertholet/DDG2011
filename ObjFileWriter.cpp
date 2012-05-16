#include "ObjFileWriter.h"
#include <iostream>
#include <sstream> 
#include <vector>
#include <fstream>
#include "tuple3.h"


ObjFileWriter::ObjFileWriter(void)
{
	
}

ObjFileWriter::~ObjFileWriter(void)
{

}

void ObjFileWriter::writeObjFile(const char * file, mesh & m, VectorField & vf)
{
	std::ofstream myFile;
	myFile.open(file);
	int internI;
	myFile << "# Obj File with arbitrary vectorfield in the normals\n";

	std::vector<tuple3f> & verts = m.getVertices();
	for(int i = 0; i < verts.size(); i++){
		myFile << "v " << verts[i].x << " " << verts[i].y << " " << verts[i].z << " \n";
	}

	tuple3f dir;
	std::vector<tuple3i> & fcs = m.getFaces();
	for(int i = 0; i < fcs.size(); i++){
		dir = vf.oneForm2Vec(i,1.f/3, 1.f/3, 1.f/3);
		myFile << "vn " << dir.x << " " << dir.y << " " << dir.z << " \n";
	}


	for(int i = 0; i < fcs.size(); i++){
		myFile << "f " << fcs[i].a +1 << "//" << i+1 << " " 
			<< fcs[i].b + 1 << "//" << i+1 << " " 
			<< fcs[i].c + 1 << "//" << i+1 << " \n";
	}

	myFile << "# eof\n";

	myFile.close();
}
