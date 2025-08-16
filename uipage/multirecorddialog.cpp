#include "multirecorddialog.h"
#include "ui_multirecorddialog.h"

MultiRecordDialog::MultiRecordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiRecordDialog)
{
    ui->setupUi(this);
    // 1. 将 "OK" 按钮的 accepted 信号，连接到对话框的 accept 槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MultiRecordDialog::accept);

    // 2. 将 "Cancel" 按钮的 rejected 信号，连接到对话框的 reject 槽
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &MultiRecordDialog::reject);
}

MultiRecordDialog::~MultiRecordDialog()
{
    delete ui;
}

void MultiRecordDialog::setLinks(const QStringList &linkIds)
{
    ui->listWidget_links->clear();
    //将字符串列表转换为带复选框的列表项
    for (const QString& id : linkIds ) {
        QListWidgetItem* item = new QListWidgetItem(id,ui->listWidget_links);
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void MultiRecordDialog::setReferenceFrames(const QStringList &frameNames)
{
    ui->comboBox_referenceFrame->clear();
    ui->comboBox_referenceFrame->addItems(frameNames);
}

QStringList MultiRecordDialog::getSelectedLinks() const
{
    QStringList selected;
    for (int i = 0; i < ui->listWidget_links->count(); ++i) {
        QListWidgetItem* item = ui->listWidget_links->item(i);
        if (item->checkState() == Qt::Checked) {
            selected.append(item->text());
        }
    }
    return selected;
}

QString MultiRecordDialog::getSelectedReferenceFrame() const
{
    return ui->comboBox_referenceFrame->currentText();
}
