#pragma once
#include "matrixf.h"
#include <vector>
#include "tuple3.h"
#include "colorMap.h"
#include <GL/glew.h>
//#include <GL/glut.h>
#include "Observer.h"

using namespace std;

enum meshMsg{POS_CHANGED, CONNECTIVITY_CHANGED};

class mesh
{
//public:

protected:
	
	//position of object in cam coordinates (cam coords = world coords)
	matrixf position;
	matrixf rotation;

	matrixf lighTransform;
		//the normals at the vertice positions
	vector<tuple3f> normals;
	//the normals to the faces.
	vector<tuple3f> face_normals;
	//the indices of the normals at the vertex points of the faces
	vector<tuple3i> face_normals_perVertex, face_tex; 
	vector<tuple3f> tex;

	// nbrs[i] is a vector with the indices of the neighbors
	//the vertices neighboring vertex i.
	vector<vector<int>> nbrs;
	//the faces neighboring vertex i.
	vector<vector<int>> nbr_fcs;
	tuple3f color;
	bool showOrientation;

	//Observer List
	vector<Observer<meshMsg> *> observer;

public:
	//observer msg
	static const int MESH_POSITIONS_CHANGED = 0;
	static const int MESH_CONNECTIVITY_CHANGED = 1;
	//the mesh
	vector<tuple3f> vertices;
	//index of the vertices of a face
	vector<tuple3i> faces; 

	mesh(void);
	mesh(const char* file);
	mesh(const char* file, tuple3f col);
	mesh(const char* file, tuple3f col, float scale);
	~mesh(void);

	void reset(vector<tuple3f> & _vertices, vector<tuple3i> & _faces);
	void initNormalsFromVertices();
	void initFaceNormals();

	void initNbrNbrfc();

	void addNormalNoise( float max );
	void scaleVertices(float scale);
	void translateVertices( float dx,float dy,float dz );

	void rotX(float phi);
	void rotY( float phi );
	void rot( float angle, float x, float y, float z );
	void scaleXYZ(float scale );
	void normalize(void);

	void move(float dz);

	vector<tuple3f>& getVertices(void){return vertices;}
	vector<tuple3i>& getFaces(){return faces;}
	vector<tuple3f> & getFaceNormals();
	// nbrs[i] is a vector with the indices of the neighbors
	//the vertices neighboring vertex i.
	vector<vector<int>> & getNeighbors() { return nbrs;}
	// nbrs[i] is a vector with the indices of the neighbors
	//the faces neighboring vertex i. Sorted by orientation
	vector<vector<int>> & getNeighborFaces(){return nbr_fcs;}
	vector<tuple3f> & getTexCoords(){return tex;}
	
	void glDisplayVertices(void);
	void glDisplayLines(void);
	void glDisplay(void);
	void glDisplay(colorMap & cMap);
	void glTexDisplay(void);
	void addUniformNoise(float sigma);
	tuple3f intensities( unsigned int faceNr, tuple3f &direction );
	tuple3f intensitiesFlat( unsigned int faceNr, tuple3f &direction );
	void setTextures_perVertex( double * x, double * y );
	void setTextures_perVertex( double * xy);
	void setTextures_perVertex( vector<tuple3f> & textures );

	/************************************************************************/
	/* returns face number hit.  start and to are supposed to already be in
	/* obj coordinates.!*/
	/************************************************************************/
	tuple3i * intersect( tuple3f & start,tuple3f &to, int * closestVertex,
		int * face, tuple3f & intersectPosition);

	void setShowOrientation(bool b){
		showOrientation = b;
	}
	void setPosition( tuple3f &pos );



	void attach(Observer<meshMsg> * o);
	void updateObserver(meshMsg msg);

private:
	void init( const char* file, tuple3f & col, float scale );

};
