#include "constraintCollector.h"
#include <algorithm>
#include "Model.h"

fieldConstraintCollector::fieldConstraintCollector(void)
{
	this->what = GUIDING_FIELD;
}

fieldConstraintCollector::~fieldConstraintCollector(void)
{
}

void fieldConstraintCollector::setWhatToCollect( collect_type sth)
{
	this->what = sth;
}

void fieldConstraintCollector::collect( int vertex)
{
	if(what== SINK_VERTS){
		if(sinkVert.size() == 0 || sinkVert.back() != vertex){
			sinkVert.push_back(vertex);
		}
	}
	else if(what== SOURCE_VERTS){
		if(sourceVert.size() == 0 || sourceVert.back() != vertex){
			sourceVert.push_back(vertex);
		}
	}
}

/*void fieldConstraintCollector::collect( int face, tuple3f & pos, tuple3f & dir )
{
	if(what == GUIDING_FIELD){
		if(faces.size() == 0 || faces.back() != face){
			tuple3f dirToPush;
			dirToPush.set(dir);
			dirToPush.normalize();

			std::vector<int>::iterator it = find(faces.begin(), faces.end(), face);
			if(it == faces.end()){
				faces.push_back(face);
				face_dir.push_back(dirToPush);
			}
			else{
				face_dir[it - faces.begin()] += dirToPush * 0.5f;
				face_dir[it - faces.begin()].normalize();
			}
		}
	}
}*/

void fieldConstraintCollector::collect( int face, int edge, tuple3f & dir )
{
	if(what == GUIDING_FIELD){
		/*if(faces.size() == 0 || faces.back() != face){*/
			tuple3f dirToPush;
			dirToPush.set(dir);
			dirToPush.normalize();

			std::vector<int>::iterator it = find(faces.begin(), faces.end(), face);
			if(it == faces.end()){
				faces.push_back(face);
				face_dir.push_back(dirToPush);
			}
			else{
				face_dir[it - faces.begin()] += dirToPush * 0.5f;
				face_dir[it - faces.begin()].normalize();
			}

			it = find(edges.begin(), edges.end(), edge);
			if(it == edges.end()){
				edges.push_back(edge);
				edge_dir.push_back(dirToPush);
			}
			else{
				edge_dir[it - edges.begin()] += dirToPush * 0.5f;
				edge_dir[it - edges.begin()].normalize();
			}
		//}
	}
}

void fieldConstraintCollector::collect(int face, tuple3i & edg, tuple3f & dir )
{
	if(what == GUIDING_FIELD){
		/*if(faces.size() == 0 || faces.back() != face){*/
		tuple3f dirToPush;
		dirToPush.set(dir);
		dirToPush.normalize();

		std::vector<int>::iterator it = find(faces.begin(), faces.end(), face);
		if(it == faces.end()){
			faces.push_back(face);
			face_dir.push_back(dirToPush);
		}
		else{
			face_dir[it - faces.begin()] += dirToPush * 0.5f;
			face_dir[it - faces.begin()].normalize();
		}

		it = find(edges.begin(), edges.end(), edg.a);
		if(it == edges.end()){
			edges.push_back(edg.a);
			edge_dir.push_back(dirToPush);
		}
		else{
			edge_dir[it - edges.begin()] += dirToPush * 0.5f;
			edge_dir[it - edges.begin()].normalize();
		}

		it = find(edges.begin(), edges.end(), edg.b);
		if(it == edges.end()){
			edges.push_back(edg.b);
			edge_dir.push_back(dirToPush);
		}
		else{
			edge_dir[it - edges.begin()] += dirToPush * 0.5f;
			edge_dir[it - edges.begin()].normalize();
		}

		it = find(edges.begin(), edges.end(), edg.c);
		if(it == edges.end()){
			edges.push_back(edg.c);
			edge_dir.push_back(dirToPush);
		}
		else{
			edge_dir[it - edges.begin()] += dirToPush * 0.5f;
			edge_dir[it - edges.begin()].normalize();
		}
		//}
	}
}

void fieldConstraintCollector::clear()
{
	sinkVert.clear();
	sourceVert.clear();
	faces.clear();
	edges.clear();
	edge_dir.clear();
	face_dir.clear();
}

void fieldConstraintCollector::glOutputConstraints( mesh * theMesh )
{
	int fc;
	std::vector<tuple3i> & fcs = theMesh->getFaces();
	std::vector<tuple3f> & vrt = theMesh->getVertices();
	std::vector<tuple3f> & fcnormals = theMesh->getFaceNormals();
	glColor3f(0.8f,0.5f,0);

	
	tuple3f pos, dir, point;
	float length = Model::getModel()->getDisplayLength();
	bool arrows = Model::getModel()->getShowArrows();
	for(int i = 0; i < faces.size(); i++){
		if(i < fcs.size()){
			glBegin(GL_LINES);
			fc = faces[i];
			pos = (vrt[fcs[fc].a]+vrt[fcs[fc].b]+vrt[fcs[fc].c]) * (1.f/3);
			pos +=  fcnormals[fc] *0.001;
			glVertex3fv( (GLfloat *) & pos);
			dir= face_dir[i] *length;
			/*dir = face_dir[i];
			dir.normalize();
			dir = dir * (0.3f);*/
			pos += dir;
			glVertex3fv( (GLfloat *) & pos);
			glEnd();

			if(arrows== true){
				glBegin(GL_TRIANGLES);
				glVertex3fv((GLfloat *) & pos) ;
				pos +=  fcnormals[fc] *0.001;
				point = fcnormals[fc].cross(dir) *(0.15f) ;
				pos = pos - dir*0.3f;
				pos = pos + point;
				glVertex3fv((GLfloat *) & pos) ;
				pos = pos + point *(-2.f);
				glVertex3fv((GLfloat *) & pos) ;
				glEnd();
			}
		}
	}
	
}

int fieldConstraintCollector::getWhat()
{

	 if(what == SOURCE_VERTS){
		 return 0;
	 }
	 if(what == SINK_VERTS){
		 return 1;
	 }if(what == GUIDING_FIELD){
		 return 2;
	 }if(what == NOTHING){
		 return 3;
	 }
}

