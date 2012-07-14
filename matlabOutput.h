#pragma once
#include <iostream>
#include <sstream> 
#include <vector>
#include <fstream>

template<class T> void saveVector( std::vector<T> & vctor, std::string name, std::string file )
{

	std::ofstream myFile;
	myFile.open(file.c_str());
	int internI;
	myFile << name << " = [";

	for(int i = 0; i < vctor.size(); i++){
		myFile << (const T&) vctor[i];
		if(i != vctor.size() -1){
			myFile << ", ";
		}
	}

	myFile << "];";

	myFile.close();
}