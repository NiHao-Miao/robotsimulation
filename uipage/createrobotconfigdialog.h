#ifndef CREATEROBOTCONFIGDIALOG_H
#define CREATEROBOTCONFIGDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QVBoxLayout>
#include "robotconfig.h"


namespace Ui {
class CreateRobotConfigDialog;
}

class QVBoxLayout;
class QSyntaxStyle;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QCompleter;
class QStyleSyntaxHighlighter;
class QCodeEditor;


class CreateRobotConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateRobotConfigDialog(QWidget *parent = nullptr,const QString& title ="" ,const RobotConfig* robot=nullptr);
    ~CreateRobotConfigDialog();

    ///
    /// \brief name  名字
    /// \return
    ///
    QString name();
    ///
    /// \brief factory 厂家
    /// \return
    ///
    QString factory();
    ///
    /// \brief description  描述
    /// \return
    ///
    QString description();

    ///
    /// \brief png   图片
    /// \return
    ///
    QString png();

    ///
    /// \brief author  作者
    /// \return
    ///
    QString author();

    ///
    /// \brief dhEnable  使用dh参数
    /// \return
    ///
    bool dhEnable();

    bool externAxisEnable();

    bool dhParamEnable();

    ///
    /// \brief stepFiles  模型文件绝对路径
    /// \return
    ///
    QStringList absStepFiles();

    ///
    /// \brief stepFiles  模型文件相对路径
    /// \return
    ///
    QStringList relStepFiles();

    ///
    /// \brief jointNumber  关节数
    /// \return
    ///
    quint32 jointNumber();

    QVector<JointInfo> jointsInfo();

    QString luaScript() const;

private slots:
    void on_add_clicked();

    void on_remove_clicked();

    void on_setpng_clicked();

    void on_delpng_clicked();

    void on_joint_number_textChanged(const QString &arg1);

    void on_edit_enable_clicked();

    void on_dh_enable_checkBox_clicked();

    void on_test_lua_clicked();

    void on_pushButton_clicked();

private:
    Ui::CreateRobotConfigDialog *ui;
    QStringListModel* step_model;
    QVBoxLayout *joint_layout;

};


#endif // CREATEROBOTCONFIGDIALOG_H
