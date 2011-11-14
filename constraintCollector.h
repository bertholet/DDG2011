#pragma once
#include <vector>

enum collect_type { SOURCE_VERTS, SINK_VERTS, NOTHING};

class fieldConstraintCollector
{
public:
	std::vector<int> sourceVert;
	std::vector<int> sinkVert;

	fieldConstraintCollector(void);
	~fieldConstraintCollector(void);


	void setWhatToCollect(collect_type);
	void collect(int val);
	void clear();
private:
	collect_type what;
};
