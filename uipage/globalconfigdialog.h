#ifndef GLOBALCONFIGDIALOG_H
#define GLOBALCONFIGDIALOG_H

#include <QDialog>


namespace Ui {
class GlobalConfigDialog;
}

class GlobalConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalConfigDialog(QWidget *parent = nullptr);
    ~GlobalConfigDialog();

    void open();
    void accept();
    void reject();

private slots:
    void on_open_last_project_clicked();

    void on_refreshCycleLineedit_editingFinished();

    void on_reset_clicked();

    void on_debug_log_checkBox_clicked();

    void on_auto_connect_checkBox_stateChanged(int arg1);

private:
    void InitConfigPara();

private:
    Ui::GlobalConfigDialog *ui;
};

#endif // GLOBALCONFIGDIALOG_H
