#include "moldingtreeview.h"
#include "ui_moldingtreeview.h"
#include <QTreeView>

MoldingTreeView::MoldingTreeView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MoldingTreeView)
{
    ui->setupUi(this);
}

MoldingTreeView::~MoldingTreeView()
{
    delete ui;
}

void MoldingTreeView::setModel(QAbstractItemModel *model)
{
    ui->treeView->setModel(model);
}
// 实现返回选择模型的方法
QItemSelectionModel *MoldingTreeView::selectionModel() const
{
    return ui->treeView->selectionModel();
}
