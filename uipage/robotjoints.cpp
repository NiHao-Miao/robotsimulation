#include "robotjoints.h"
#include "ui_robotjoints.h"
#include "jointcontrol.h"
#include "robotbase.h"


RobotJoints::RobotJoints(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RobotJoints),
    axis_num(0)
{
    ui->setupUi(this);
//    robot_base = new RobotBase(this);
//    ui->verticalLayout->addWidget(robot_base);
}

RobotJoints::~RobotJoints()
{
    delete ui;
}

void RobotJoints::setAxisNum(int num)
{
    for(int i = 0;i<joint_controls.size();i++){
        delete joint_controls[i];
    }
    joint_controls.clear();
    axis_num = num;
    for(int i = 0;i<num;i++){
        JointControl* joint = new JointControl();
        joint->setName("关节"+QString::number(i+1));
        joint->setRange(-360,360,3);
        joint_controls.append(joint);
        connect(joint,&JointControl::valueChanged,this,[this](){
            QVector<double> pos;
            for(int i = 0;i<joint_controls.size();i++){
                pos.append(joint_controls.at(i)->value());
            }
            emit valueChange(pos);
        });
        ui->verticalLayout->addWidget(joint);
    }
}

bool RobotJoints::setJointsValue(const QVector<double> &pos)
{
    for(int i = 0;i<pos.size();i++){
        if(i >= joint_controls.size())
            return true;
        joint_controls.at(i)->setValue(pos[i]);
    }
    return false;
}

void RobotJoints::setJointRange(int id, double max, double min, int dec)
{
    if(id >= joint_controls.size())
        return;
    joint_controls[id]->setRange(min,max,dec);
}

void RobotJoints::on_origin_btn_clicked()
{
    QVector<double> pos;
    for(int i = 0;i<joint_controls.size();i++){
        joint_controls.at(i)->setValue(0);
        pos.append(joint_controls.at(i)->value());
    }
    emit valueChange(pos);
}


