#pragma once

/************************************************************************/
/* Extremely basic Observer implementation. Allows Observed objects to 
call update with an int msg*/
/************************************************************************/
class Observer
{
public:
	Observer(void);
	~Observer(void);

	virtual void update(void * src, int type) = 0;
};
