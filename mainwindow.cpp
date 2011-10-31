#include "mainwindow.h"
#include <iostream>
#include "mesh.h"

MainWindow::MainWindow(): QMainWindow()
{
	this->setWindowTitle(
		QApplication::translate("toplevel", "Top-level widget"));

	this->fileMenu = menuBar()->addMenu("File");
	this->openObjFileAct = new QAction("Open Obj File...", this);
	this->generateMeshAct = new QAction("Genereate Mesh", this);
	this->fileMenu->addAction(openObjFileAct);
	this->fileMenu->addAction(generateMeshAct);

	connect(openObjFileAct, SIGNAL(triggered()), this, SLOT(openObjFile()));

	myGLDisp = new Displayer(this);

	comboBox = new QComboBox();
	comboBox->setEditable(false);
	comboBox->addItem("Lines");
	comboBox->addItem("Faces");
	comboBox->addItem("Border");
	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setDisplayMode(int)));

	//comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(myGLDisp);
	layout->addWidget(comboBox);
	layout->setStretchFactor(comboBox,0);


	QWidget * mainWidget = new QWidget();
	mainWidget->setLayout(layout);
	this->setCentralWidget(mainWidget);

	this->resize(500, 500);
	this->show();
}


MainWindow::~MainWindow()
{

}

void MainWindow::openObjFile()
{
	std::cout << "objFileLoad Clicked";
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Obj. File"), "/home/", tr("Obj Files (*.obj)"));
	
	if(!fileName.endsWith("obj")){
		QMessageBox msgBox;
		msgBox.setText("Not an Obj file");
		return;
	}

	mesh * amesh = new mesh(fileName.toAscii());
	amesh->normalize();
	Model::getModel()->setMesh(amesh);
	this->update();
}

void MainWindow::update()
{
	this->myGLDisp->updateGL();
}


void MainWindow::setDisplayMode( int mode )
{
	if(mode == 0){
		this->myGLDisp->setMode(EDGEMODE);
	}
	else if(mode == 1){
		this->myGLDisp->setMode(FLATMODE);
	}
	else if(mode == 2){
		this->myGLDisp->setMode(FLATMODE);
	}
}
