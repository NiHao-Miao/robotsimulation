#include "historyversionlistdialog.h"
#include "ui_historyversionlistdialog.h"
#include "utils/autoupdate.h"
#include <QFile>

HistoryVersionListDialog::HistoryVersionListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryVersionListDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("更新日志");
    QFile file("CHANGELOG");
    if(file.open(QFile::ReadWrite)){
        this->ui->textBrowser->setText(file.readAll());
        file.close();
    }
    else{
        this->ui->textBrowser->setText(AutoUpdate::Instance()->getHistoryVersionList());
    }
}

HistoryVersionListDialog::~HistoryVersionListDialog()
{
    delete ui;
}
