#pragma once
#include <vector>
#include "tuple3.h"
#include "mesh.h"
#include "VectorField.h"

enum collect_type { SOURCE_VERTS, SINK_VERTS, GUIDING_FIELD, NOTHING};

class fieldConstraintCollector
{
public:
	std::vector<int> sourceVert;
	std::vector<int> sinkVert;

	fieldConstraintCollector(void);
	~fieldConstraintCollector(void);


	void setWhatToCollect(collect_type);
	void collect(int val);
	//void collect( int face, tuple3f & pos , tuple3f & dir );
	void collect(int face, int edge, tuple3f & dir );
	void collect(int face, tuple3i & edges, tuple3f & dir );

	void clear();
	void glOutputConstraints( mesh * theMesh );
	
	std::vector<int> & getEdges(){
		return edges;
	}
	std::vector<tuple3f> & getEdgeDirs(){
		return edge_dir;
	}

private:
	collect_type what;
	std::vector<int> edges;
	std::vector<tuple3f> face_dir;
	std::vector<tuple3f> edge_dir;
	std::vector<int> faces;
};
