#pragma once
#include "pardisomatcreator.h"
#include <algorithm>
#include <vector>

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


//////////////////////////////////////////////////////////////////////////
// Will create a matrix with ones on the specified diagonal elements
//////////////////////////////////////////////////////////////////////////
class sparseDiagCreator: public pardisoMatCreator
{

private:
	std::vector<std::vector<int>>  * ones;
public:

	sparseDiagCreator(std::vector<std::vector<int>>  * _ones){
		ones = _ones;
	}

	float val(int i , int j){
		// i is the row
		if(i==j){

			for(int k = 0; k < ones->size(); k++){
				if(find((*ones)[k].begin(),(*ones)[k].end(),i) != (*ones)[k].end()){
					return 1;
				}
			}
		}
		return 0;
	}

	// row: its the edge number; 
	void indices(int row, std::vector<int> & target){
		target.clear();
		target.push_back(row);
	}
};