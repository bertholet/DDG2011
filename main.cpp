#include "mainwindow.h"
#include <QtGui>
#include "mesh.h"
#include "TutteWeights.h"
#include "TutteDemo.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow window;

/*	mesh * bunny= new mesh("C:/Users/bertholet/Dropbox/workspace/RA/objfiles/bunny5k.obj", tuple3f(1.f,0.f,0.f), 2);
	bunny->normalize();
	
	TutteDemo demo;
	demo.run_multiBorder(*bunny, TutteWeights::cotan_weights_divAvor);

	delete bunny;
	return 0;*/
	return app.exec();

}
