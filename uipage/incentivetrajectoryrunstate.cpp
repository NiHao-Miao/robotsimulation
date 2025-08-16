#include "incentivetrajectoryrunstate.h"
#include "ui_incentivetrajectoryrunstate.h"

IncentiveTrajectoryRunState::IncentiveTrajectoryRunState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IncentiveTrajectoryRunState)
{
    ui->setupUi(this);
    ui->progressBar->setRange(0,100);
    onProgressUpdate(0);
}

IncentiveTrajectoryRunState::~IncentiveTrajectoryRunState()
{
    delete ui;
}

void IncentiveTrajectoryRunState::onProgressUpdate(double value)
{
    ui->progressBar->setValue(value*100);
}

void IncentiveTrajectoryRunState::onRunTimeUpdate(uint32_t value)
{

}

void IncentiveTrajectoryRunState::on_x1_clicked()
{
    emit magnificationChanged(1);
}


void IncentiveTrajectoryRunState::on_x5_clicked()
{
    emit magnificationChanged(5);
}


void IncentiveTrajectoryRunState::on_x10_clicked()
{
    emit magnificationChanged(10);
}


void IncentiveTrajectoryRunState::on_x20_clicked()
{
    emit magnificationChanged(20);
}


void IncentiveTrajectoryRunState::on_play_clicked()
{
    emit start();
}


void IncentiveTrajectoryRunState::on_pause_clicked()
{
    emit pause();
}


void IncentiveTrajectoryRunState::on_stop_clicked()
{
    emit stop();
}

