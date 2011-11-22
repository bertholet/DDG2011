#pragma once

/************************************************************************/
/* Extremely basic Observer implementation. Allows Observed objects to 
call update with an int msg*/
/************************************************************************/
template <typename _enum>
class Observer
{
public:
	Observer(void){}
	virtual ~Observer(void){}

	virtual void update(void * src, _enum msg) = 0;
};
