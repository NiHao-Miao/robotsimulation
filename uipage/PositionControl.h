#pragma once

#include <QWidget>
#include "ui_PositionControl.h"

class PositionControl : public QWidget
{
	Q_OBJECT

public:
	PositionControl(QWidget *parent = nullptr);
	~PositionControl();

private:
	Ui::PositionControlClass ui;
};
