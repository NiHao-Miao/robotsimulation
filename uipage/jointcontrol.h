#ifndef JOINTCONTROL_H
#define JOINTCONTROL_H

#include <QWidget>

namespace Ui {
class JointControl;
}

class JointControl : public QWidget
{
    Q_OBJECT

public:
    explicit JointControl(QWidget *parent = nullptr);
    ~JointControl();

    void setName(const QString& text);

    void setRange(double min,double max,int dec);

    void setValue(double value);

    double value();

    QSize sizeHint() const;

signals:
    void valueChanged(double value);

private slots:
    void onValueChanged(int value);

private:
    Ui::JointControl *ui;

    int dec;
};

#endif // JOINTCONTROL_H
