#include "mainwindow.h"
#include <QtGui>
#include "mesh.h"
//#include "TutteWeights.h"
//#include "TutteDemo.h"
#include "pardisoMatrix.h"

#include "matrixf.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow window;


/*	mesh * bunny= new mesh("C:/Users/bertholet/Dropbox/workspace/RA/objfiles/bunny5k.obj", tuple3f(1.f,0.f,0.f), 2);
	bunny->normalize();//
	
	TutteDemo demo;
	demo.run_multiBorder(*bunny, TutteWeights::cotan_weights_divAvor);

	delete bunny;
	return 0;*/

	int ia[] = {1,2,3,4,5,7};
	int ja[] = {1,2,3,4,1,6};
	double a[] = {2,3,1,4,1,3};
	pardisoMatrix A(ia,ja,a,6,6);
	int ia2[] = {1,3,4,5,7,8,9};
	int ja2[] = {1,2,2,3,3,4,5,6};
	double a2[] = {3,1,1,4,3,2,1,2};
	pardisoMatrix A2(ia2,ja2,a2,7,8);

	pardisoMatrix C = A*A2;


	return app.exec();

}
