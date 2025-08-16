#ifndef JOINTSETTING_H
#define JOINTSETTING_H

#include <QWidget>

namespace Ui {
class JointSetting;
}

class JointSetting : public QWidget
{
    Q_OBJECT

public:
    explicit JointSetting(QWidget *parent = nullptr);
    ~JointSetting();


    void setName(const QString& name);

signals:
    void updatePos(const QVector<double>& pos);

public slots:
    void on_update_clicked();


private:
    Ui::JointSetting *ui;
};

#endif // JOINTSETTING_H
