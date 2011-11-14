#ifndef VECTORFIELDCONTROLWIDGET_H
#define VECTORFIELDCONTROLWIDGET_H

#include <QWidget>

class vectorFieldControlWidget : public QWidget
{
	Q_OBJECT

public:
	vectorFieldControlWidget(QWidget *parent = 0);
	~vectorFieldControlWidget();

private:
	
private slots:
	void genAxisAllignedField();
	void solveVField();
	void sourceSelection( bool active );
	void sinkSelection( bool active );
	void fieldSelection( bool active );
};

#endif // VECTORFIELDCONTROLWIDGET_H
