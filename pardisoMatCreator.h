#pragma once
#include "stdafx.h"
#include <vector>

class pardisoMatCreator{
public:
	virtual ~pardisoMatCreator(){}

	//returns matrix value at theis position.
	virtual float val(int i , int j)=0;
	virtual void indices(int row, std::vector<int> & target)=0;
}; 