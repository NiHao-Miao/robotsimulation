#ifndef ROBOTBASE_H
#define ROBOTBASE_H

#include <QWidget>

namespace Ui {
class RobotBase;
}

class RobotBase : public QWidget
{
    Q_OBJECT

public:
    explicit RobotBase(QWidget *parent = nullptr);
    ~RobotBase();

    void setBaseJoints(const QVector<double>& pos);


signals:
    void updateBase(const QVector<double>& pos);
    void updateTool(const QVector<double>& pos);


private slots:
    void on_updatebase_clicked();

    void on_updatetool_clicked();

private:
    Ui::RobotBase *ui;
};

#endif // ROBOTBASE_H
