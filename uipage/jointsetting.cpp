#include "jointsetting.h"
#include "ui_jointsetting.h"

JointSetting::JointSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JointSetting)
{
    ui->setupUi(this);
}

JointSetting::~JointSetting()
{
    delete ui;
}

void JointSetting::setName(const QString& name) {
    ui->groupBox->setTitle(name);
}

void JointSetting::on_update_clicked()
{
    QVector<double> pos;
    pos.append(ui->x->text().toDouble());
    pos.append(ui->y->text().toDouble());
    pos.append(ui->z->text().toDouble());
    pos.append(ui->u->text().toDouble());
    pos.append(ui->v->text().toDouble());
    pos.append(ui->w->text().toDouble());
    emit updatePos(pos);
}

