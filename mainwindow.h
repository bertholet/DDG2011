#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

private:
	QMenu * fileMenu;
	QAction * openObjFileAction;
	QAction * generateMeshAction;

};

#endif // MAINWINDOW_H
