#include "mystatusbar.h"
#include <QVBoxLayout>
#include <QLabel>

myStatusBar::myStatusBar(QWidget *parent /*= 0*/ )
	: QDialog(parent)
{
	QVBoxLayout * whatLayout = new QVBoxLayout();

	this->myBar = new QProgressBar(this);
	whatLayout->addWidget(new QLabel("Setting up Sparse Matrix: "));
	whatLayout->addWidget(myBar);
	this->setLayout(whatLayout);

	connect(this, SIGNAL(updateMyBar(int)), myBar, SLOT(setValue(int)));
}

myStatusBar::~myStatusBar()
{

}

void myStatusBar::setBar( int min, int max )
{
	this->myBar->setRange(min, max);
}

void myStatusBar::updateBar( int step )
{
	emit updateMyBar(step);
}
