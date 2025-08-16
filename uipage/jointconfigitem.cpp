#include "jointconfigitem.h"
#include "ui_jointconfigitem.h"

JointConfigItem::JointConfigItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JointConfigItem)
{
    ui->setupUi(this);
}

JointConfigItem::~JointConfigItem()
{
    delete ui;
}

void JointConfigItem::init(int id,const JointInfo &info)
{
    ui->groupBox->setTitle("关节"+QString::number(id));
    ui->model_id->setText(QString::number(info.axis));
    ui->model_x->setText(QString::number(info.x));
    ui->model_y->setText(QString::number(info.y));
    ui->model_z->setText(QString::number(info.z));
    if(info.dir==-1){
        ui->dir->setCurrentIndex(1);
    }
    else{
        ui->dir->setCurrentIndex(0);
    }
    ui->type->setCurrentIndex(info.type);
    ui->p_limit->setText(QString::number(info.p_limit));
    ui->n_limit->setText(QString::number(info.n_limit));
    ui->default_origin->setText(QString::number(info.default_origin));
    ui->dh_alpha->setText(QString::number(info.dh.alpha));
    ui->dh_a->setText(QString::number(info.dh.a));
    ui->dh_d->setText(QString::number(info.dh.d));
    ui->dh_theta->setText(QString::number(info.dh.theta));
    ui->max_speed->setText(QString::number(info.max_speed));
}

JointInfo JointConfigItem::getInfo() const
{
    JointInfo info;
    info.axis = ui->model_id->text().toInt();
    info.x = ui->model_x->text().toDouble();
    info.y = ui->model_y->text().toDouble();
    info.z = ui->model_z->text().toDouble();
    info.p_limit = ui->p_limit->text().toDouble();
    info.n_limit = ui->n_limit->text().toDouble();
    info.type = ui->type->currentIndex();
    info.default_origin = ui->default_origin->text().toDouble();
    info.max_speed = ui->max_speed->text().toDouble();
    if(ui->dir->currentIndex() == 0){
        info.dir = 1;
    }
    else{
        info.dir = -1;
    }
    info.dh.alpha = ui->dh_alpha->text().toDouble();
    info.dh.a = ui->dh_a->text().toDouble();
    info.dh.d = ui->dh_d->text().toDouble();
    info.dh.theta = ui->dh_theta->text().toDouble();
    return info;
}
