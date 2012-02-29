#include "StdAfx.h"
#include "pardisoMatrix.h"
#include <assert.h>
#include "idCreator.h"

pardisoMatrix::pardisoMatrix(void)
{
	n= 0;
	m= 0;
}

pardisoMatrix::pardisoMatrix( int * ia_, int *ja_,
							 double * a_, int sz_ia, int sz_ja)
{
	n= 0;
	m= 0;
	for(int i = 0; i < sz_ia; i++){
		this->iapush_back(ia_[i]);
	}
	for(int j = 0; j< sz_ja; j++){
		this->japush_back(ja_[j]);
		this->apush_back(a_[j]);
	}
}

pardisoMatrix::~pardisoMatrix(void)
{
}

void pardisoMatrix::initMatrix( pardisoMatCreator & creator, int n , myStatusBar * bar )
{
	std::vector<int> js;
	int j;
	int stp = n/100 +1;
	int max = (n - n%stp)/stp;
	int nrStp = 0;
	if(bar != NULL){
		bar->setBar(0,max);
	}

	this->clear();

	for(int i = 0; i < n ; i++){
		creator.indices(i,js);
		iapush_back(a.size() +1); //stupid one based notation...

		for(int k = 0; k < js.size(); k++){
			j = js[k];
			apush_back(creator.val(i,j));
			japush_back(j+1);
		}

		if(bar != NULL && i % stp == 0){
			bar->updateBar(nrStp++);
		}


	}
	iapush_back(a.size() +1);
}

void pardisoMatrix::saveMatrix( std::string file )
{
	std::ofstream myFile;
	int internI;
	bool element_n_m_missing = (ia[ia.size()-1] - ia[ia.size()-2] == 0 ||
		 ja[ja.size()-1]!=m);
	//bool lastRowDegenerated = (ia[ia.size()-1] - ia[ia.size()-2] == 0);

	myFile.open(file.c_str());

	myFile << "i= [";
	//internI is the row number
	internI = 0;
	for(int i = 0; i < a.size(); i++){
		//i goes over all indices. i.e determine for each a[i] its "i" matrix
		//coordinate. ia[internI+1] is the start index of 0 based row internI
		while(i+1 >= ia[internI+1]){
			internI++;
			myFile << "...\n";
		}
		myFile << boost::lexical_cast<std::string>(internI+1);
		if(i!= a.size() -1){
			myFile <<", ";
		}
	}
	if(element_n_m_missing){
		myFile << ",...\n " << boost::lexical_cast<std::string>(n);
	}

	myFile << "];\n\n j=[";

	internI = 0;
	for(int i = 0; i < a.size(); i++){
		while(i+1 >= ia[internI+1]){
			internI++;
			myFile << "...\n";
		}
		myFile << boost::lexical_cast<std::string>(ja[i]);
		if(i!= a.size() -1){
			myFile <<", ";
		}
	}
	if(element_n_m_missing){
		myFile << ",...\n " << boost::lexical_cast<std::string>(m);
	}

	myFile << "];\n\n a=[";
	internI = 0;
	for(int i = 0; i < a.size(); i++){
		while(i+1 >= ia[internI+1]){
			internI++;
			myFile << "...\n";
		}
		myFile << boost::lexical_cast<std::string>(a[i]);
		if(i!= a.size() -1){
			myFile <<", ";
		}
	}
	if(element_n_m_missing){
		myFile << ",...\n " << boost::lexical_cast<std::string>(0);
	}
	myFile << "];";

	myFile.close();
}

void pardisoMatrix::saveVector(std::vector<double> & vctor, std::string  name, 
							   std::string  file )
{
	std::ofstream myFile;
	int internI;
	myFile.open(file.c_str());
	myFile << name << " = [";

	for(int i = 0; i < vctor.size(); i++){
		myFile << vctor[i];
		if(i != vctor.size() -1){
			myFile << ", ";
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

void pardisoMatrix::add( int i, int j, float val )
{
	int bs = ia[i]-1;
	bool added = false;
	for(int k = bs; k < ia[i+1]-1; k++){
		if(ja[k] == j+1){
			a[k]+= val;
			added = true;
			break;
		}
	}

	if(!added){
		assert(false);
		throw std::runtime_error("Error in pardisoMatrix::add : (i,j) not a Matrix Entry");
	}
}

void pardisoMatrix::addLine(std::vector<int> & js, std::vector<double> & vals){
	assert(js.size()  == vals.size());
	for(int i = 0; i < js.size();i++){
		a.push_back(vals[i]);
		japush_back(js[i]+1);
	}
	iapush_back(a.size() +1);

}

pardisoMatrix pardisoMatrix::operator*( pardisoMatrix & B )
{
	assert(B.getn() == this->getm());
	pardisoMatrix AB;
	AB.ia.reserve(this->dim());
	AB.ja.reserve(3*this->dim());
	AB.a.reserve(3*this->dim());
	AB.iapush_back(1);

	int Aia_start, Aia_stop, next_j, k;
	double val;
	std::vector<int> b_idx, b_stop;

	for(int i = 0; i < this->n; i++){
		//new this.ia[i]
		Aia_start = this->ia[i]-1;
		Aia_stop = this->ia[i+1]-1;

		//reset b_idx and b_stop
		b_idx.clear();
		b_stop.clear();
		for(int l = Aia_start; l < Aia_stop; l++){
			k=this->ja[l]-1; //loop k s.t. A(i,k) != 0;
			b_idx.push_back(B.ia[k]-1); //index of first val in row k for matrix B
										//j = B.ja[b_idx] first j such that B(k,j)!=0;
			b_stop.push_back(B.ia[k+1]-1); //at b_stop the next row starts.
		}

		//b_idx contains now the indices of the values in B.a of the rows k  with A(i,k) != 0
		//has size Aia_stop-Aia_start.

		//loop the j such that A*B(i,j)!=0
		while(true){
			//find next j value : the first column with unconsidered values
			//i.e. will multiply row i of A with column next_j
			next_j = B.getm()+2; //invalid index for break condition
			for(int l = 0; l < Aia_stop-Aia_start; l++){
				//value not considered and in a column earlier than next_j
				if( b_idx[l] < b_stop[l] && next_j >B.ja[b_idx[l]]){
					next_j = B.ja[b_idx[l]];
				}
			}

			//break condition
			if(next_j > B.getm()+1) 
				break;

			//calculate A*B(i,next_j)
			val = 0;
			for(int l=Aia_start,l2 = 0; l < Aia_stop; l++,l2++){
				//"B(k,next_j)!=0" i.e. multiplicating row i with column j
				if(b_idx[l2] < b_stop[l2]&&B.ja[b_idx[l2]] == next_j){
					val+=this->a[l]*B.a[b_idx[l2]];
					b_idx[l2]++;//this row is done. Advance to next non zero row
				}
			}

			if(val!= 0)
			{
				//store values
				AB.japush_back(next_j);
				AB.apush_back(val);

			}
		}

		//adapt AB.ia
		AB.iapush_back(AB.a.size()+1);

		
	}
		
	AB.forceNrColumns(B.getm());

	return AB;
}


pardisoMatrix pardisoMatrix::operator%( pardisoMatrix & B )
{
	assert(B.getm() == this->getm());
	bool emptyLineFound = false;
	pardisoMatrix AB;

	AB.ia.reserve(this->dim());
	AB.ja.reserve(3*this->dim());
	AB.a.reserve(3*this->dim());
	AB.iapush_back(1);

	int Aia_start, Aia_stop, next_j, k;
	double val;
//	std::vector<int> b_idx, b_stop;
	int Bia_start, Bia_stop;

	for(int i = 0; i < dim(); i++){
		//new this.ia[i]
		Aia_start = this->ia[i]-1;
		Aia_stop = this->ia[i+1]-1;

		//loop the j such that A*B(i,j)!=0
		for(int j = 0; j < B.dim(); j++){
			//find next j value
			next_j = j+1; 
			Bia_start = B.ia[j] -1;
			Bia_stop = B.ia[j+1] -1; // first index of next row

			if(this->ja[Aia_start] > B.ja[Bia_stop-1] || 
				this->ja[Aia_stop-1] < B.ja[Bia_start]){
					continue;
			}

			//calculate (A*B^T)(i,next_j)
			val = 0;
			for(int l=Aia_start, l2 = Bia_start; l < Aia_stop && l2 < Bia_stop;){
				//"B(k,next_j)!=0"
				while(B.ja[l2] < this->ja[l] && l2 < Bia_stop){
					l2++;
				}
				while(this->ja[l] <B.ja[l2] && l < Aia_stop){
					l++;
				}

				if(B.ja[l2] == this->ja[l] && l<Aia_stop && l2 < Bia_stop){
					val+=this->a[l]*B.a[l2];
					l2++;
					l++;
				}
			}

			if(val!=0){
				//store values
				AB.japush_back(next_j);
				AB.a.push_back(val);
			}
		}


		//adapt AB.ia
		if(AB.ia.back() == AB.a.size()+1){
			emptyLineFound = true;
		}
		else if( !emptyLineFound){
			AB.iapush_back(AB.a.size()+1);
		}
		else{
			//cout << "Matrix degenerated!in pardisoMatrix%";
			assert(false);
			throw std::runtime_error("Matrix degenerated in pardisoMatrix::%");
		}


	}


	return AB;
}

pardisoMatrix pardisoMatrix::operator+( pardisoMatrix & B )
{
	assert(getn() == B.getn() && getm() == B.getm());
	pardisoMatrix AnB;
	AnB.ia.reserve(this->dim());
	AnB.ja.reserve(3*this->dim());
	AnB.a.reserve(3*this->dim());
	AnB.iapush_back(1);
	
	int Aia_start, Aia_stop, Bia_start, Bia_stop;
	int j1, j2;
	float val;
	
	for(int i = 0; i < dim(); i++){
		Aia_start = this->ia[i]-1;
		Aia_stop = this->ia[i+1]-1;
		Bia_start = B.ia[i]-1;
		Bia_stop = B.ia[i+1]-1;
		for(j1 = Aia_start, j2 = Bia_start; j1 <Aia_stop || j2 < Bia_stop;){
			if(j2 >= Bia_stop || this->ja[j1]< B.ja[j2] && j1 < Aia_stop){
				val = this->a[j1];
				if(val!=0){
					AnB.japush_back(this->ja[j1]);
					AnB.apush_back(val);
				}
				j1++;
			}
			else if (j1 >= Aia_stop || this->ja[j1]> B.ja[j2] && j2 < Bia_stop){
				val = B.a[j2];
				if(val!= 0){
					AnB.japush_back(B.ja[j2]);
					AnB.apush_back(val);
				}
				j2++;
			}
			else{
				val = B.a[j2] + this->a[j1];
				if(val != 0){
					AnB.japush_back(B.ja[j2]);
					AnB.apush_back(val);
				}
				j1++;
				j2++;
			}

		} 	

		//adapt AB.ia
		AnB.iapush_back(AnB.a.size()+1);
	}
	return AnB;
}


pardisoMatrix pardisoMatrix::operator-( pardisoMatrix & B )
{
	assert(getn() == B.getn() && getm() == B.getm());
	pardisoMatrix AnB;
	AnB.ia.reserve(this->dim());
	AnB.ja.reserve(3*this->dim());
	AnB.a.reserve(3*this->dim());
	AnB.iapush_back(1);

	int Aia_start, Aia_stop, Bia_start, Bia_stop;
	int j1, j2;
	float val;

	for(int i = 0; i < dim(); i++){
		Aia_start = this->ia[i]-1;
		Aia_stop = this->ia[i+1]-1;
		Bia_start = B.ia[i]-1;
		Bia_stop = B.ia[i+1]-1;
		for(j1 = Aia_start, j2 = Bia_start; j1 <Aia_stop || j2 < Bia_stop;){
			if(this->ja[j1]< B.ja[j2] && j1 < Aia_stop || j2 >= Bia_stop){

				val = this->a[j1];
				if(val!=0){
					AnB.japush_back(this->ja[j1]);
					AnB.apush_back(val);
				}
				j1++;
			}
			else if (this->ja[j1]> B.ja[j2] && j2 < Bia_stop || j1 >= Aia_stop){
				val = - B.a[j2];
				if(val!= 0){
					AnB.japush_back(B.ja[j2]);
					AnB.apush_back(val);
				}
				j2++;
			}
			else{
				val = -B.a[j2] + this->a[j1];
				if(val!= 0){
					AnB.japush_back(B.ja[j2]);
					AnB.apush_back(val);
				}
				j1++;
				j2++;
			}

		} 	

		//adapt AB.ia
		AnB.iapush_back(AnB.a.size()+1);
	}
	return AnB;
}


void pardisoMatrix::operator*=( float  other )
{
	for(int i = 0; i < a.size(); i++){
		a[i]*=other;
	}
}



void pardisoMatrix::elementWiseInv(double eps)
{
	for(int i = 0; i < a.size(); i++){
		if(a[i]> eps || a[i]< -eps){
			a[i] = 1/ a[i];
		}
	}
}

void pardisoMatrix::clear()
{
	a.clear();
	ia.clear();
	ja.clear();
	n = 0;
	m= 0;
}

void pardisoMatrix::mult( std::vector<double> & x, std::vector<double> & target, bool adaptTarget )
{
	assert(x.size() == getm() && (adaptTarget|| target.size() == getn()));
	assert(&target != &x);

	if(adaptTarget){
		target.clear();
		target.reserve(getn());
	}
	for(int i = 0; i < getn(); i++){
		if(adaptTarget){
			target.push_back(0);
		}
		target[i] = 0;
		for(int j = ia[i]-1; j < ia[i+1]-1; j++){
			target[i] += x[ja[j]-1]*a[j];
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// set line line to 0.... 1 ... 0 where line is the line in 0 based notation
//////////////////////////////////////////////////////////////////////////
void pardisoMatrix::setLineToID( int line )
{
	bool diagElementExisted = false;
	for(int j = ia[line]-1; j < ia[line+1]-1; j++){
		if(ja[j] -1 == line){
			a[j] = 1;
			diagElementExisted = true;
		}
		else{
			a[j] = 0;
		}
	}

	assert(diagElementExisted);
	if(!diagElementExisted){
		throw std::runtime_error( "Unimplemented Case in setLineToID(), curse the lazy programmer who preferred to write this error msg than to implement a better method");
	}
	//was lazy and did not yet implement this method if the diag element does not exist.

}

pardisoMatrix pardisoMatrix::transpose( pardisoMatrix & mat )
{
	pardisoMatrix id;
	id.initMatrix(idCreator(),mat.m);
	return id % mat;
}

//////////////////////////////////////////////////////////////////////////
// force the number columns to have nrColums size.
// This might be needed if you want to assure the dimension of the matrix
// are n x nrColumns when the last columns are empty. ( this sparse format
// would ignore empty trailing columns)
void pardisoMatrix::forceNrColumns( int nrColumns )
{
	assert(m <=nrColumns); // else the matrix will behave strangely..
	m= nrColumns;
}


