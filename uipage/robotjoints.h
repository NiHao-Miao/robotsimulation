#ifndef ROBOTJOINTS_H
#define ROBOTJOINTS_H

#include <QWidget>

namespace Ui {
class RobotJoints;
}

class JointControl;
class RobotBase;

class RobotJoints : public QWidget
{
    Q_OBJECT

public:
    explicit RobotJoints(QWidget *parent = nullptr);
    ~RobotJoints();

    void setAxisNum(int num);

    bool setJointsValue(const QVector<double>& pos);

    ///
    /// \brief setJointRange    设置关节范围
    /// \param id
    /// \param max
    /// \param min
    /// \param dec
    ///
    void setJointRange(int id,double max,double min,int dec);

signals:
    void valueChange(QVector<double>);

private slots:
    void on_origin_btn_clicked();

private:
    Ui::RobotJoints *ui;
    RobotBase* robot_base;

    int axis_num;
    QVector<JointControl*> joint_controls;
};

#endif // ROBOTJOINTS_H
