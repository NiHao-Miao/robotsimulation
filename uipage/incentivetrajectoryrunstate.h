#ifndef INCENTIVETRAJECTORYRUNSTATE_H
#define INCENTIVETRAJECTORYRUNSTATE_H

#include <QWidget>

namespace Ui {
class IncentiveTrajectoryRunState;
}

class IncentiveTrajectoryRunState : public QWidget
{
    Q_OBJECT

public:
    explicit IncentiveTrajectoryRunState(QWidget *parent = nullptr);
    ~IncentiveTrajectoryRunState();

signals:
    void magnificationChanged(double value);
    void start();
    void stop();
    void pause();

public slots:
    void onProgressUpdate(double value);
    void onRunTimeUpdate(uint32_t value);

private slots:
    void on_x1_clicked();

    void on_x5_clicked();

    void on_x10_clicked();

    void on_x20_clicked();

    void on_play_clicked();

    void on_pause_clicked();

    void on_stop_clicked();

private:
    Ui::IncentiveTrajectoryRunState *ui;
};

#endif // INCENTIVETRAJECTORYRUNSTATE_H
