#pragma once
#include <vector>
#include "tuple3.h"
#include "mesh.h"

enum collect_type { SOURCE_VERTS, SINK_VERTS, GUIDING_FIELD, NOTHING};

class fieldConstraintCollector
{
public:
	std::vector<int> sourceVert;
	std::vector<int> sinkVert;
	std::vector<int> faces;
	std::vector<tuple3f> face_dir;

	fieldConstraintCollector(void);
	~fieldConstraintCollector(void);


	void setWhatToCollect(collect_type);
	void collect(int val);
	void collect( int face, tuple3f & dir );
	void clear();
	void glOutputConstraints( mesh * theMesh );
private:
	collect_type what;
};
