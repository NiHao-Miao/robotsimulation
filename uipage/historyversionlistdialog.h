#ifndef HISTORYVERSIONLISTDIALOG_H
#define HISTORYVERSIONLISTDIALOG_H

#include <QDialog>

namespace Ui {
class HistoryVersionListDialog;
}

class HistoryVersionListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryVersionListDialog(QWidget *parent = nullptr);
    ~HistoryVersionListDialog();

private:
    Ui::HistoryVersionListDialog *ui;
};

#endif // HISTORYVERSIONLISTDIALOG_H
