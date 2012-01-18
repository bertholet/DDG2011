#pragma once
#include "stdafx.h"
#include <vector>

class pardisoMatCreator{
public:
	virtual ~pardisoMatCreator(){}

	//////////////////////////////////////////////////////////////////////////
	//returns matrix value at theis position.
	// 0 based indices are assumed, they will converted to 1 based indices 
	// when used to init a pardisomatrix automatically
	//////////////////////////////////////////////////////////////////////////
	virtual float val(int i , int j)=0;

	//////////////////////////////////////////////////////////////////////////
	// stores a list of all indices j (0 based) in target, that contains all indices with
	// val(row,j) != 0. val(row, j) = 0 is allowed. Sorted indices.
	//////////////////////////////////////////////////////////////////////////
	virtual void indices(int row, std::vector<int> & target)=0;
}; 