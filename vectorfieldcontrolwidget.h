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

};

#endif // VECTORFIELDCONTROLWIDGET_H
