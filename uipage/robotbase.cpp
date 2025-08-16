#include "robotbase.h"
#include "ui_robotbase.h"

RobotBase::RobotBase(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RobotBase)
{
    ui->setupUi(this);
}

RobotBase::~RobotBase()
{
    delete ui;
}

void RobotBase::setBaseJoints(const QVector<double> &pos)
{
    if(pos.size() != 6)
        return;
    ui->base_x->setText(QString::number(pos[0]));
    ui->base_y->setText(QString::number(pos[1]));
    ui->base_z->setText(QString::number(pos[2]));
    ui->base_u->setText(QString::number(pos[3]));
    ui->base_v->setText(QString::number(pos[4]));
    ui->base_w->setText(QString::number(pos[5]));
    emit updateBase(pos);
}

void RobotBase::on_updatebase_clicked()
{
    QVector<double> pos;
    pos.append(ui->base_x->text().toDouble());
    pos.append(ui->base_y->text().toDouble());
    pos.append(ui->base_z->text().toDouble());
    pos.append(ui->base_u->text().toDouble());
    pos.append(ui->base_v->text().toDouble());
    pos.append(ui->base_w->text().toDouble());
    emit updateBase(pos);
}


void RobotBase::on_updatetool_clicked()
{
    QVector<double> pos;
    pos.append(ui->tool_x->text().toDouble());
    pos.append(ui->tool_y->text().toDouble());
    pos.append(ui->tool_z->text().toDouble());
    pos.append(ui->tool_u->text().toDouble());
    pos.append(ui->tool_v->text().toDouble());
    pos.append(ui->tool_w->text().toDouble());
    emit updateTool(pos);
}

