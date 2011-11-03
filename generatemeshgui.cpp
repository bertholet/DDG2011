#include "generatemeshgui.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include "ball.h"
#include "Model.h"
#include "torus.h"
#include "cube.h"

GenMeshDialog::GenMeshDialog( QWidget *parent /*= 0*/ ):QDialog(parent)
{
	whatCBox = new QComboBox(this);
	whatCBox->setEditable(false);
	whatCBox->addItem("Sphere");
	whatCBox->addItem("Torus");
	whatCBox->addItem("Cube");
	whatCBox->addItem("Irregular Sphere");

	connect(whatCBox,SIGNAL(currentIndexChanged(int)),this, SLOT(cBoxSelectAct(int)));

	QLabel * typeLabel = new QLabel("Mesh Type:",this);
	QPushButton * okButton = new QPushButton("OK", this);
	okButton->setDefault(true);
	connect(okButton,SIGNAL(released()), this, SLOT(okButtonAct()));

	inputlabel1 = new QLabel("", this);
	inputlabel2 = new QLabel("", this);
	inputlabel3 = new QLabel("", this);

	input1 = new QSpinBox(this);
	input1->setMinimum(5);
	input1->setMaximum(1000);
	input1->setSingleStep(5);
	input2 = new QSpinBox(this);
	input2->setMinimum(2);
	input2->setMaximum(1000);
	input2->setSingleStep(5);
	input3 = new QSpinBox(this);
	input3->setMinimum(5);
	input3->setMaximum(1000);
	input3->setSingleStep(5);

	QHBoxLayout * whatLayout = new QHBoxLayout();
	whatLayout->addWidget(typeLabel,0);
	whatLayout->addWidget(whatCBox,1);

	QGridLayout * dialogLayout = new QGridLayout();
	dialogLayout->addLayout(whatLayout,0,0,1,2);

	dialogLayout->addWidget(inputlabel1,1,1,1,1);
	dialogLayout->addWidget(input1,1,2,1,2);
	dialogLayout->addWidget(inputlabel2,2,1,1,1);
	dialogLayout->addWidget(input2,2,2,1,2);
	dialogLayout->addWidget(inputlabel3,3,1,1,1);
	dialogLayout->addWidget(input3,3,2,1,2);

	dialogLayout->addWidget(okButton,4,3,1,1);

	QGridLayout * inputLayout = new QGridLayout();
	

	this->setLayout(dialogLayout);
	this->inputSphereTorus();
//	resize(300,300);
	
}

GenMeshDialog::~GenMeshDialog()
{
}

void GenMeshDialog::cBoxSelectAct( int nr)
{
	if (nr == 0){
		this->inputSphereTorus();
	}
	else if (nr == 1){
		this->inputSphereTorus();
	}
	else if (nr == 2){
		this->inputCube();
	}
	else if (nr == 3){
		this->inputIrregularSphere();
	}
}

void GenMeshDialog::hideInput()
{
	this->inputlabel1->setText("");
	this->inputlabel2->setText("");
	this->inputlabel3->setText("");

	this->input1->hide();
	this->input2->hide();
	this->input3->hide();
}

void GenMeshDialog::inputSphereTorus()
{
	this->inputlabel1->setText("phi");
	this->inputlabel2->setText("psi");
	this->inputlabel3->setText("");
	this->input1->show();
	this->input2->show();
	this->input3->hide();
}

void GenMeshDialog::inputCube()
{
	this->inputlabel1->setText("Vertices per side:");
	this->inputlabel2->setText("");
	this->inputlabel3->setText("");
	this->input1->show();
	this->input2->hide();
	this->input3->hide();
}

void GenMeshDialog::inputIrregularSphere()
{
	this->inputlabel1->setText("phi top");
	this->inputlabel2->setText("phi bottom");
	this->inputlabel3->setText("psi");
	this->input1->show();
	this->input2->show();
	this->input3->show();
}

void GenMeshDialog::okButtonAct()
{
	int what = whatCBox->currentIndex();
	mesh *m;
	if(what == 0){
		m = new ball(1,input1->value(), input2->value());
		m->normalize();
		Model::getModel()->setMesh(m);
	}
	else if(what ==1){
		m = new torus(2,1,input1->value(), input2->value());
		m->normalize();
		Model::getModel()->setMesh(m);
	}
	else if(what ==2){
		m = new cube(1,input1->value());
		m->normalize();
		Model::getModel()->setMesh(m);
	}
	else if(what ==3){
		m = new ball(1,input1->value(),input2->value(),input3->value());
		m->normalize();
		Model::getModel()->setMesh(m);
	}
	done(0);
}

