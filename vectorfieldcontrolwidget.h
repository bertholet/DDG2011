#ifndef VECTORFIELDCONTROLWIDGET_H
#define VECTORFIELDCONTROLWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include "Observer.h"
#include "Model.h"
#include "VectorFieldSolver.h"
#include "mainwindow.h"

class vectorFieldControlWidget : public QWidget, public Observer<Model::modelMsg>
{
	Q_OBJECT

public:
	vectorFieldControlWidget(QWidget *parent = 0);
	~vectorFieldControlWidget();

	void setMainWindow(MainWindow *);
private:
	VectorFieldSolver * solver;
	QSlider * gfWeihgtSlider;
	MainWindow * mainWindow;
	QSlider * flowSlider;
	QCheckBox * cBoxDirectional;

	int weightStep, srcFlowStep;
	QSlider * gfLengthSlider;
	int lengthStep;
	int weightMax;
	void initSolver();
private slots:
	void genAxisAllignedField();
	void solveVField();
	void sourceSelection( bool active );
	void sinkSelection( bool active );
	void fieldSelection( bool active );

	virtual void update( void * src, Model::modelMsg msg );

};

#endif // VECTORFIELDCONTROLWIDGET_H
