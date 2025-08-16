#include "attachmentdialog.h"
#include "ui_attachmentdialog.h"

AttachmentDialog::AttachmentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttachmentDialog)
{
    ui->setupUi(this);
}

AttachmentDialog::~AttachmentDialog()
{
    delete ui;
}

// ---：接口的实现 ---
void AttachmentDialog::setAttachmentTargets(const QStringList& targetNames)
{
    ui->comboBox_attachmentTarget->clear();
    ui->comboBox_attachmentTarget->addItems(targetNames);
}

QString AttachmentDialog::getSelectedTarget() const
{
    return ui->comboBox_attachmentTarget->currentText();
}

void AttachmentDialog::setTrackableLinks(const QStringList& linkIds)
{
    ui->comboBox_trackedLink->clear();
    ui->comboBox_trackedLink->addItems(linkIds);
    // 默认选中最后一个，通常是 "Tool" 或 "TCP"，这是最常用的
    if (!linkIds.isEmpty()) {
        ui->comboBox_trackedLink->setCurrentIndex(linkIds.count() - 1);
    }
}

QString AttachmentDialog::getSelectedLink() const
{
    return ui->comboBox_trackedLink->currentText();
}
