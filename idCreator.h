#pragma once
#include "pardisomatcreator.h"

class idCreator: public pardisoMatCreator
{

public:

	float val(int i , int j){
		// i is the row
		if(i==j){
			return 1;
		}
		return 0;
	}

	// row: its the edge number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		target.push_back(row);
	}
};
