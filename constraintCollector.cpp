#include "constraintCollector.h"

fieldConstraintCollector::fieldConstraintCollector(void)
{
	this->what = NOTHING;
}

fieldConstraintCollector::~fieldConstraintCollector(void)
{
}

void fieldConstraintCollector::setWhatToCollect( collect_type sth)
{
	this->what = sth;
}

void fieldConstraintCollector::collect( int val )
{
	if(what== SINK_VERTS){
		sinkVert.push_back(val);
	}
	else if(what== SOURCE_VERTS){
		sourceVert.push_back(val);
	}
}

void fieldConstraintCollector::clear()
{
	sinkVert.clear();
	sourceVert.clear();
}
