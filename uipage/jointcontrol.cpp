#include "jointcontrol.h"
#include "ui_jointcontrol.h"

JointControl::JointControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JointControl)
{
    ui->setupUi(this);
    connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(onValueChanged(int)));
    setMinimumSize(sizeHint());
    dec  =  0;
    ui->value->setText(QString::number(this->value()));
}

JointControl::~JointControl()
{
    delete ui;
}

void JointControl::setName(const QString &text)
{
    ui->name->setText(text);
}

void JointControl::setRange(double min, double max, int dec)
{
    ui->horizontalSlider->setRange(min*pow(10,dec),max*pow(10,dec));
    this->dec = dec;
}

void JointControl::setValue(double value)
{
    ui->horizontalSlider->setValue(value*pow(10,dec));
}

double JointControl::value()
{
    return (double)ui->horizontalSlider->value()/pow(10,dec);
}

QSize JointControl::sizeHint() const
{
    return QSize(200,50);
}

void JointControl::onValueChanged(int value)
{
    ui->value->setText(QString::number(this->value()));
    emit valueChanged((double)value/pow(10,dec));
}
