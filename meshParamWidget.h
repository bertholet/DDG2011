#pragma once
#include <QWidget>

class meshParamWidget : public QWidget
{
	Q_OBJECT
public:
	meshParamWidget(QWidget *parent = NULL);
	~meshParamWidget(void);

	int outerMode;
	int weightMode;

public slots:
	void conformalBorder(void);
	void neumannBorder( void );
	void circleBorder(void);
	void setBorderMode( int mode );
	void setWeightMode( int mode );
};
