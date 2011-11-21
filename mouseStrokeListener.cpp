#include "mouseStrokeListener.h"
#include "matrixf.h"
#include "Model.h"
#include <vector>

mouseStrokeListener::mouseStrokeListener(triangleMarkupMap * _map, QGLWidget *_daddy)
{
	this->map = _map;
	this->daddy = _daddy;
	this->nrCalls = 0;
}

mouseStrokeListener::~mouseStrokeListener(void)
{
}

void mouseStrokeListener::onMouseMove( QMouseEvent* event )
{
	GLdouble proj[16], model[16], Z;
	GLint viewPort[4];
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, viewPort);

	float winx = (float) event->x() , 
		winy = (float) (viewPort[3] - event->y()), winz;

	/*glReadPixels(winy, winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Z );
	winz = (float) Z;*/

	GLdouble p1x,p1y,p1z;
	tuple3f start, end;

	gluUnProject(winx, winy,-1, model,proj,viewPort,&p1x,&p1y,&p1z);
	start.x = (float) p1x;
	start.y = (float) p1y;
	start.z = (float) p1z;

	gluUnProject(winx, winy,0, model,proj,viewPort,&p1x,&p1y,&p1z);
	end.x = (float) p1x;
	end.y = (float) p1y;
	end.z = (float) p1z;


	int vertex, face;
	tuple3f next;
	tuple3i * fc = Model::getModel()->getMesh()->intersect(start,end, &vertex, &face, next);

	if(fc != NULL){
		//this->map->mark(*fc, nrCalls);
		this->map->mark(vertex, nrCalls);
		this->daddy->updateGL();

		Model::getModel()->getInputCollector().collect(vertex);

		if(lastValid){
		/*	int edge = edgeCrossed(face, last, next - last);*/
			tuple3i & edge = edges(face);
			Model::getModel()->getInputCollector().collect(face,edge, next - last);
		}
		last.set(next);
		lastValid = true;
	}
	else{
		lastValid = false;
	}
}

void mouseStrokeListener::onMousePress( QMouseEvent* event )
{
	nrCalls++;
	lastValid = false;
	onMouseMove(event);

}

tuple3i & mouseStrokeListener::edges(int faceNr){
	return (*(Model::getModel()->getMeshInfo()->getFace2Halfedges()))[faceNr];
}

int mouseStrokeListener::edgeCrossed( int faceNr, tuple3f & pos, tuple3f & dir )
{
	tuple3i fc = Model::getModel()->getMesh()->getFaces()[faceNr];
	tuple3i & f2e = (*(Model::getModel()->getMeshInfo()->getFace2Halfedges()))[faceNr];
	std::vector<tuple3f> & verts = Model::getModel()->getMesh()->getVertices();
	tuple3f fcNormal = verts[fc.b] - verts[fc.a];
	fcNormal = -fcNormal.cross(verts[fc.c]-verts[fc.a]);
	fcNormal.normalize();

	tuple3f dir_cross_normal = dir.cross(fcNormal);
	dir_cross_normal.normalize();

	float d = dir_cross_normal.dot(pos);
	float a = (verts[fc.a]).dot(dir_cross_normal) -d;
	float b = (verts[fc.b]).dot(dir_cross_normal) -d;
	float c = (verts[fc.c]).dot(dir_cross_normal) -d;

	if(a<= 0 && b>=0){
		return f2e.a;
	}
	else if(b<=0 && c>=0){
		return f2e.b;
	}
	else if(c<=0 && a>=0){
		return f2e.c;
	}
	else{
		assert(false);
		throw runtime_error("Error in Mousestrokelistened:: Edge resolve");
		return -1;
	}

}
