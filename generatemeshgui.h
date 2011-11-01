#ifndef GENERATEMESHGUI_H
#define GENERATEMESHGUI_H

#include <QDialog>
#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QSpinBox>

class GenMeshDialog : public QDialog
{
	Q_OBJECT

public:
	GenMeshDialog(QWidget *parent = 0);
	~GenMeshDialog();

	void hideInput();
private:
	QComboBox * whatCBox;
	QLabel * inputlabel1;
	QLabel * inputlabel2;
	QLabel * inputlabel3;
	QSpinBox * input1;
	QSpinBox * input2;
	QSpinBox * input3;

	void inputSphereTorus();
	void inputIrregularSphere();
	void inputCube();

private slots:
	void cBoxSelectAct(int);
	void okButtonAct();
};

#endif // GENERATEMESHGUI_H
