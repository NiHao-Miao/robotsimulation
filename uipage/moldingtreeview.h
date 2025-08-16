#ifndef MOLDINGTREEVIEW_H
#define MOLDINGTREEVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

namespace Ui {
class MoldingTreeView;
}

class MoldingTreeView : public QWidget
{
    Q_OBJECT

public:
    explicit MoldingTreeView(QWidget *parent = nullptr);
    ~MoldingTreeView();

    void setModel(QAbstractItemModel *model);
//    添加一个公共方法以返回内部treeView的选择模型
    QItemSelectionModel* selectionModel() const;
private:
    Ui::MoldingTreeView *ui;
};

#endif // MOLDINGTREEVIEW_H
