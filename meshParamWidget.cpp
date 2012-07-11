#include "meshParamWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "TutteEmbedding.h"
#include "TutteWeights.h"
#include "Model.h"
#include <QComboBox>


meshParamWidget::meshParamWidget(QWidget *parent)
: QWidget(parent)
{
	outerMode =0;
	weightMode = 0;
	QPushButton * butt = new QPushButton("One Circle Border");
	connect(butt, SIGNAL(released()), this, SLOT(circleBorder()));
	QPushButton * butt2 = new QPushButton("Conformal Borders");
	connect(butt2, SIGNAL(released()), this, SLOT(conformalBorder()));
	QLabel * label1 = new QLabel("Outer Border: ");
	QLabel * label2 = new QLabel("Weights: ");

	QComboBox * comboBox = new QComboBox();
	comboBox->setEditable(false);
	comboBox->addItem("Circle");
	comboBox->addItem("Circle Weighted");
	comboBox->addItem("Conformal");
	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setBorderMode(int)));


	QComboBox * comboBox2 = new QComboBox();
	comboBox2->setEditable(false);
	comboBox2->addItem("Cotan Voronoi");
	comboBox2->addItem("Cotan Mixed");
	comboBox2->addItem("Mean Value");
	comboBox2->addItem("Uniform");
	connect(comboBox2, SIGNAL(currentIndexChanged(int)), this, SLOT(setWeightMode(int)));

	QVBoxLayout * layout = new QVBoxLayout();
	layout->addWidget(label1);
	layout->addWidget(comboBox);
	layout->addWidget(label2);
	layout->addWidget(comboBox2);
	layout->addWidget(butt);
	layout->addWidget(butt2);
	this->setLayout(layout);
}

meshParamWidget::~meshParamWidget(void)
{
}

void meshParamWidget::circleBorder( void )
{
	TutteEmbedding embedding;

	mesh * myMesh = Model::getModel()->getMesh();
	int nrBorders = Model::getModel()->getMeshInfo()->getBorder().size();

	if(myMesh != NULL && nrBorders == 1){
		embedding.calcTexturePos(*myMesh);
	}
}

void meshParamWidget::conformalBorder( void )
{
	TutteEmbedding embedding;

	mesh * myMesh = Model::getModel()->getMesh();
	int nrBorders = Model::getModel()->getMeshInfo()->getBorder().size();

	double (*weights ) (int, int,
		mesh &,
		vector<int>& /*nbr_i*/,
		vector<int>&/*fc_i*/,
		vector<int>& /*border*/);
	void (* borderFunc)( vector<tuple3f> & /*outerPos*/ , vector<int> & /*border*/, mesh &);

	if(weightMode == 0){
		weights= TutteWeights::cotan_weights_divAvor;
	}
	else if(weightMode == 1){
		weights= TutteWeights::cotan_weights_divAmix;
	}
	else if(weightMode == 2){
		weights= TutteWeights::unnormed_meanvalue_weights;
	}
	else{
		weights= TutteWeights::uniform_weights;
	}

	if(outerMode == 0){
		borderFunc = TutteWeights::circleBorder;
	}
	else if(outerMode == 1){
		borderFunc = TutteWeights::distWeightCircBorder;
	}
	else{
		borderFunc = TutteWeights::angleApproxBorder;
	}
	if(myMesh != NULL && nrBorders >0){
		embedding.calcTexturePos_multiBorder(*myMesh,weights,borderFunc );//TutteWeights::unnormed_meanvalue_weights);
	}
}

void meshParamWidget::setBorderMode( int mode )
{
	//throw std::exception("The method or operation is not implemented.");
	outerMode = mode;
}

void meshParamWidget::setWeightMode( int mode )
{
	weightMode = mode;
	//throw std::exception("The method or operation is not implemented.");
}
