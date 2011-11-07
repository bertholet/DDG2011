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
	QTabWidget * tabs;

	QCheckBox * cbox;
	QPushButton * butt;

	void setupQTabs();
	void setupMenubar() ;
	void setupButtons();

	void addAction();
	void layoutGui();

private slots:
	void openObjFile();
	void generateMesh();
	void setDisplayMode(int);
	void resetStrokes();

	void setMouseMode(int state);
};

#endif // MAINWINDOW_H
