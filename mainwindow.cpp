#include "mainwindow.h"


MainWindow::MainWindow(): QMainWindow()
{
	this->resize(320, 240);
	this->show();
	this->setWindowTitle(
		QApplication::translate("toplevel", "Top-level widget"));

	fileMenu = menuBar()->addMenu("File");
	openObjFileAction = new QAction("Open Obj File...", this);
	generateMeshAction = new QAction("Genereate Mesh", this);
	fileMenu->addAction(openObjFileAction);
	fileMenu->addAction(generateMeshAction);

}


MainWindow::~MainWindow()
{

}
