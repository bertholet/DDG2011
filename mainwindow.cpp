#include "mainwindow.h"


MainWindow::MainWindow(): QMainWindow()
{
	this->resize(320, 240);
	this->show();
	this->setWindowTitle(
		QApplication::translate("toplevel", "Top-level widget"));


}


MainWindow::~MainWindow()
{

}
