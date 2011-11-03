#include "vectorfieldcontrolwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>

vectorFieldControlWidget::vectorFieldControlWidget(QWidget *parent)
	: QWidget(parent)
{
	QPushButton *butt = new QPushButton("Generate VField!");
	connect(butt, SIGNAL(released()), this, SLOT(genAxisAllignedField()));

	QVBoxLayout * layout = new QVBoxLayout();
	layout->addWidget(butt);

	this->setLayout(layout);
}

vectorFieldControlWidget::~vectorFieldControlWidget()
{

}

void vectorFieldControlWidget::genAxisAllignedField()
{
	std::cout << "click";
}
