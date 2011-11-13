#include "StdAfx.h"
#include "pardisoMatrix.h"

pardisoMatrix::pardisoMatrix(void)
{
}

pardisoMatrix::~pardisoMatrix(void)
{
}

void pardisoMatrix::initMatrix( pardisoMatCreator & creator, int dim )
{
	std::vector<int> js;
	int j;

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
