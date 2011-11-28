#ifndef MYSTATUSBAR_H
#define MYSTATUSBAR_H

#include <QDialog>
#include <QProgressBar>

class myStatusBar : public QDialog
{
	Q_OBJECT

public:
	myStatusBar(QWidget *parent = 0);
	~myStatusBar();

	void setBar(int min, int max);
	void updateBar(int step);

private:
	QProgressBar * myBar;

signals:
	void updateMyBar(int step);
};

#endif // MYSTATUSBAR_H
