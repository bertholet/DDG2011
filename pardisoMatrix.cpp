#include "StdAfx.h"
#include "pardisoMatrix.h"
#include <assert.h>

pardisoMatrix::pardisoMatrix(void)
{
}

pardisoMatrix::~pardisoMatrix(void)
{
}

void pardisoMatrix::initMatrix( pardisoMatCreator & creator, int dim , myStatusBar * bar )
{
	std::vector<int> js;
	int j;
	int stp = dim/100 +1;
	int max = (dim - dim%stp)/stp;
	int nrStp = 0;
	if(bar != NULL){
		bar->setBar(0,max);
	}

	ia.clear();
	ja.clear();
	a.clear();

	for(int i = 0; i < dim ; i++){
		creator.indices(i,js);
		ia.push_back(a.size() +1); //stupid one based notation...

		for(int k = 0; k < js.size(); k++){
			j = js[k];
			a.push_back(creator.val(i,j));
			ja.push_back(j+1);
		}

		if(bar != NULL && i % stp == 0){
			bar->updateBar(nrStp++);
		}


	}
	ia.push_back(a.size() +1);
}

void pardisoMatrix::saveMatrix( std::string file )
{
	std::ofstream myFile;
	int internI;
	myFile.open(file.c_str());

	myFile << "i= [";
	internI = 0;
	for(int i = 0; i < a.size(); i++){
		if(i+1 >= ia[internI+1]){
			internI++;
			myFile << "...\n";
		}
		myFile << boost::lexical_cast<std::string>(internI+1);
		if(i!= a.size() -1){
			myFile <<", ";
		}
	}

	myFile << "];\n\n j=[";

	internI = 0;
	for(int i = 0; i < a.size(); i++){
		if(i+1 >= ia[internI+1]){
			internI++;
			myFile << "...\n";
		}
		myFile << boost::lexical_cast<std::string>(ja[i]);
		if(i!= a.size() -1){
			myFile <<", ";
		}
	}

	myFile << "];\n\n a=[";
	internI = 0;
	for(int i = 0; i < a.size(); i++){
		if(i+1 >= ia[internI+1]){
			internI++;
			myFile << "...\n";
		}
		myFile << boost::lexical_cast<std::string>(a[i]);
		if(i!= a.size() -1){
			myFile <<", ";
		}
	}
	myFile << "];";

	myFile.close();
}

int pardisoMatrix::dim()
{
	return ia.size()-1;
}

void pardisoMatrix::getDiagonalIndices( std::vector<int> & target_ind )
{
	target_ind.clear();
	target_ind.reserve(ia.size()-1);
	bool hasDiagEl;

	for(int i = 0; i <ia.size()-1; i++){
		hasDiagEl = false;
		for(int j = 0; j < ia[i+1]-ia[i]; j++){
			if(ja[ia[i] - 1 + j] == i+1) { //stupid one-based notation
				hasDiagEl = true;
				target_ind.push_back(ia[i] + j -1);
			}
		}
		assert(hasDiagEl);
		assert(ja[target_ind.back()] == i+1); //i really do hate one based stuff.
	}
}
