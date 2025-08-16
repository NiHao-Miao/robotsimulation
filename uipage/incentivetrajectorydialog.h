#ifndef INCENTIVETRAJECTORYDIALOG_H
#define INCENTIVETRAJECTORYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>

namespace Ui {
class IncentiveTrajectoryDialog;
}

class IncentiveTrajectoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IncentiveTrajectoryDialog(QWidget *parent = nullptr);
    ~IncentiveTrajectoryDialog();

    ///
    /// \brief toString  数据转字符串
    /// \return
    ///
    QString toString() const;

    ///
    /// \brief import   字符串导入
    /// \param str
    ///
    void import(const QString& str,bool show_err = false);

private slots:
    void on_exportButton_clicked();

    void on_pushButton_clicked();

    void on_importButton_clicked();

private:
    std::vector<double> optimize();

private:
    Ui::IncentiveTrajectoryDialog *ui;
    QVBoxLayout *joint_layout;
    QVector<QVector<QLineEdit*> > joint_para;
};

#endif // INCENTIVETRAJECTORYDIALOG_H
