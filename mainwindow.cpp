#include "mainwindow.h"
#include <iostream>
#include <QCheckBox>
#include <QPushButton>
#include "mesh.h"
#include "curvVisualizingMesh.h"
#include "generatemeshgui.h"
#include "vectorfieldcontrolwidget.h"

MainWindow::MainWindow(): QMainWindow()
{
	setupMenubar();
	setupButtons();
	setupQTabs();

	addAction();


	layoutGui();

	this->show();
}


MainWindow::~MainWindow()
{

}

/************************************************************************/
/* The menubar                                                                     */
/************************************************************************/
void MainWindow::setupMenubar() 
{
	this->fileMenu = menuBar()->addMenu("File");
	this->openObjFileAct = new QAction("Open Obj File...", this);
	this->generateMeshAct = new QAction("Genereate Mesh", this);
	this->fileMenu->addAction(openObjFileAct);
	this->fileMenu->addAction(generateMeshAct);
}

/************************************************************************/
/* Seeting up Buttons, Qcombobox, etc                                   */
/************************************************************************/
void MainWindow::setupButtons() 
{
	myGLDisp = new Displayer(this);

	comboBox = new QComboBox();
	comboBox->setEditable(false);
	comboBox->addItem("Lines");
	comboBox->addItem("Faces");
	comboBox->addItem("Border");
	comboBox->addItem("Curvature");
	comboBox->addItem("Selections");

	cbox = new QCheckBox("Draw strokes",this);
	butt = new QPushButton("Reset", this);
}

/************************************************************************/
/* Sets up the Contol elements in the Qtabpanel                         */
/************************************************************************/
void MainWindow::setupQTabs() 
{
	this->tabs = new QTabWidget(this);
	QWidget * tab1Widget = new QWidget();
	tabs->addTab(tab1Widget, "Smoothing");
	vectorFieldControlWidget * tab2Widget = new vectorFieldControlWidget();
	tabs->addTab(tab2Widget, "Vector Fields");
}

/************************************************************************/
/* All the connec methods                                               */
/************************************************************************/
void MainWindow::addAction() 
{

	connect(openObjFileAct, SIGNAL(triggered()), this, SLOT(openObjFile()));
	connect(generateMeshAct,SIGNAL(triggered()), this, SLOT(generateMesh()));
	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setDisplayMode(int)));
	connect(cbox, SIGNAL(stateChanged(int)), this, SLOT(setMouseMode(int)));
	connect(butt, SIGNAL(released()), this, SLOT(resetStrokes()));
}

/************************************************************************/
/* Layout all the components                                            */
/************************************************************************/
void MainWindow::layoutGui() 
{
	//layout the gui
	QVBoxLayout * rightLayout = new QVBoxLayout();
	rightLayout->addWidget(comboBox);
	rightLayout->addWidget(tabs);
	QHBoxLayout * sublayout = new QHBoxLayout();
	sublayout->addWidget(cbox);
	sublayout->addWidget(butt);
	rightLayout->addLayout(sublayout);

	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->addWidget(myGLDisp,1);
	mainLayout->addLayout(rightLayout,0);

	QWidget * mainWidget = new QWidget();
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

	this->setWindowTitle(
		QApplication::translate("toplevel", "DG2011"));
	this->resize(600, 500);
}


/************************************************************************/
/* to Update the GL display in this component                           */
/************************************************************************/
void MainWindow::update()
{
	this->myGLDisp->updateGL();
}

/************************************************************************/
/* Takies care of the open obj file Action                                                                     */
/************************************************************************/
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

//////////////////////////////////////////////////////////////////////////
//The Slots.
//////////////////////////////////////////////////////////////////////////


/************************************************************************/
/* Takes care of the generate Mesh action.                              */
/************************************************************************/
void MainWindow::generateMesh()
{
	GenMeshDialog genDialog(this);
	genDialog.exec();
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
	else if(mode == 3){
		Model & model = *Model::getModel();
		//model.getMeshInfo()->activateCurvNormals(true);
		this->myGLDisp->setColormap((colorMap *) new curvColormap(* Model::getModel()->getMesh()));
		this->myGLDisp->setMode(COLORMAPMODE);
	}
	else if(mode == 4){
		Model & model = *Model::getModel();
		//model.getMeshInfo()->activateCurvNormals(true);
		this->myGLDisp->setMode(MOUSEINPUTMODE);
	}
}

void MainWindow::setMouseMode( int state )
{
	if(state == 0){//unchecked
		this->myGLDisp->setMouseMode(TRACKBALLMODE);
	}
	if(state == 2){//checked
		this->myGLDisp->setMouseMode(INPUTMODE);
		this->myGLDisp->setMode(MOUSEINPUTMODE);
	}
}

void MainWindow::resetStrokes()
{
	this->myGLDisp->resetStrokes();
	Model::getModel()->getInputCollector().clear();
}
