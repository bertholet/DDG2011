#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Model.h"
#include <QtGui>
#include "displayer.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

	void update();

private:
	QMenu * fileMenu;
	QAction * openObjFileAct;
	QAction * generateMeshAct;

	Displayer * myGLDisp;
	QComboBox * comboBox;
private slots:
	void openObjFile();
	void setDisplayMode(int);
};

#endif // MAINWINDOW_H
