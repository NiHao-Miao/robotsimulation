#ifndef INCENTIVETRAJECTORYTHREAD_H
#define INCENTIVETRAJECTORYTHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QTime>

class IncentiveTrajectoryThread : public QObject
{
    Q_OBJECT
public:
    explicit IncentiveTrajectoryThread(QObject *parent = nullptr);

public slots:
    void start();
    void pause();
    void stop();

    ///
    /// \brief setSpeedMultiple  设置运行倍率
    /// \param value
    ///
    void setSpeedMultiple(double value);

public:
    ///
    /// \brief setPosList       设位置列表
    /// \param pos
    ///
    void setPosList(const QVector<QVector<double>>& pos);
signals:
    void updateRobotJoints(QVector<double>);

    ///
    /// \brief updateProgress  更新运行进度
    /// \param value
    ///
    void updateProgress(double value);

    ///
    /// \brief updateRunTime  更新运行时间
    /// \param time
    ///
    void updateRunTime(uint32_t time);

    ///
    /// \brief runFinished      运行完成
    ///
    void runFinished();

private:
    QTimer timer;
    uint32_t pos_index;
    QVector<QVector<double>> pos_list;
    uint32_t speed;
    QTime run_time;
};

#endif // INCENTIVETRAJECTORYTHREAD_H
