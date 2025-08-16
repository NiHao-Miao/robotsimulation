#ifndef JOINTCONFIGITEM_H
#define JOINTCONFIGITEM_H

#include <QWidget>
#include "robotconfig.h"


namespace Ui {
class JointConfigItem;
}

class JointConfigItem : public QWidget
{
    Q_OBJECT

public:
    explicit JointConfigItem(QWidget *parent = nullptr);
    ~JointConfigItem();

    void init(int id, const JointInfo& info);

    JointInfo getInfo() const;

private:
    Ui::JointConfigItem *ui;
};

#endif // JOINTCONFIGITEM_H
