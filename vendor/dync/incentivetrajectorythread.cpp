#include "incentivetrajectorythread.h"

const uint32_t initial_speed = 50;

IncentiveTrajectoryThread::IncentiveTrajectoryThread(QObject *parent) : QObject(parent),pos_index(0)
{
    speed = initial_speed;
    timer.setInterval(initial_speed);
    timer.setSingleShot(false);
    connect(&timer,&QTimer::timeout,this,[this](){
        if(pos_index < pos_list.size()){
            updateRobotJoints(pos_list[pos_index]);
            emit updateProgress((double)pos_index/pos_list.size());
            pos_index+=speed;
        }
        else{
            this->stop();
            emit updateProgress(1);
            emit runFinished();
        }

    });
}

void IncentiveTrajectoryThread::start()
{
    timer.start();
}

void IncentiveTrajectoryThread::pause()
{
    timer.stop();
}

void IncentiveTrajectoryThread::stop()
{
    timer.stop();
    pos_index = 0;
    speed = initial_speed;
}

void IncentiveTrajectoryThread::setSpeedMultiple(double value)
{
    speed = initial_speed*value;
}

void IncentiveTrajectoryThread::setPosList(const QVector<QVector<double> > &pos)
{
    pos_list = pos;
    pos_index = 0;
}

