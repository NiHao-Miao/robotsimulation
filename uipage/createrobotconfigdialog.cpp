#include "createrobotconfigdialog.h"
#include "ui_createrobotconfigdialog.h"
#include "jointconfigitem.h"
#include "qdebug.h"
#include <QFileDialog>
#include "luathread.h"
#include "QCodeEditor.hpp"
#include <QGLSLCompleter.hpp>
#include <QLuaCompleter.hpp>
#include <QPythonCompleter.hpp>
#include <QSyntaxStyle.hpp>
#include <QCXXHighlighter.hpp>
#include <QGLSLHighlighter.hpp>
#include <QXMLHighlighter.hpp>
#include <QJSONHighlighter.hpp>
#include <QLuaHighlighter.hpp>
#include <QPythonHighlighter.hpp>

CreateRobotConfigDialog::CreateRobotConfigDialog(QWidget *parent, const QString &title, const RobotConfig *robot) :
    QDialog(parent),
    ui(new Ui::CreateRobotConfigDialog)
{
    ui->setupUi(this);
    step_model = new QStringListModel();
    ui->stepfiles->setModel(step_model);
    if(title == ""){
        this->setWindowTitle("创建机器人仿真模型");
    }
    else{
        this->setWindowTitle(title);
    }
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 设置水平滚动条的显示策略
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 设置垂直滚动条的显示策略
    ui->scrollArea->setWidgetResizable(true); // 当内容大小发生变化时，自动调整滚动区域的大小

    joint_layout = new QVBoxLayout(ui->scrollAreaWidgetContents);

    if(robot!= nullptr){
        ui->name->setEnabled(true);
        ui->edit_enable->show();
//        ui->gridLayout->setEnabled(false);
        ui->model_tab->setEnabled(false);
        ui->joint_tab->setEnabled(false);
        ui->lua_tab->setEnabled(false);
        ui->scrollAreaWidgetContents->setEnabled(false);
//        ui->horizontalLayout_2->setEnabled(false);

        ui->name->setText(robot->name());
        ui->factory->setText(robot->factory());
        ui->description->setText(robot->description());
        ui->png->setText(robot->png());
        ui->author->setText(robot->author());
        ui->joint_number->setText(QString::number(robot->jointNumber()));
        step_model->setStringList(robot->stepFiles());
        int count = joint_layout->count();
        for(int i  = 0;i<count;i++){
            auto item =static_cast<JointConfigItem*>(joint_layout->itemAt(i)->widget());
            item->init(i+1,robot->getJointInfo(i));
        }
        ui->dh_enable_checkBox->setChecked(robot->getDhEnable());
        ui->luatextEdit->setText(robot->getLuaScript());
        ui->dh_param_checkbox->setChecked(robot->getDhParamEnable());
        ui->externAxis_checkBox->setChecked(robot->getExternAxis());
    }
    else{
        ui->edit_enable->hide();
    }

    ui->plainTextEdit->setSyntaxStyle(QSyntaxStyle::defaultStyle());
    ui->plainTextEdit->setCompleter  (new QLuaCompleter(this));
    ui->plainTextEdit->setHighlighter(new QLuaHighlighter);

    ui->luatextEdit->setSyntaxStyle(QSyntaxStyle::defaultStyle());
    ui->luatextEdit->setCompleter  (new QLuaCompleter(this));
    ui->luatextEdit->setHighlighter(new QLuaHighlighter);

}


CreateRobotConfigDialog::~CreateRobotConfigDialog()
{
    delete ui;
}


QStringList CreateRobotConfigDialog::absStepFiles()
{
    return step_model->stringList();
}

QStringList CreateRobotConfigDialog::relStepFiles()
{
    QStringList new_step_files;
    for(int i = 0;i<step_model->stringList().size();i++){
        QFileInfo file(step_model->stringList()[i]);
        new_step_files.append(file.fileName());
    }
    return new_step_files;
}

quint32 CreateRobotConfigDialog::jointNumber()
{
    return ui->joint_number->text().toUInt();
}

QVector<JointInfo> CreateRobotConfigDialog::jointsInfo()
{
    QVector<JointInfo> infos;
    for(int i = 0;i<joint_layout->count();i++){
        infos.append(static_cast<JointConfigItem*>(joint_layout->itemAt(i)->widget())->getInfo());
    }
    return infos;
}

QString CreateRobotConfigDialog::luaScript() const
{
    return ui->luatextEdit->toPlainText();
}

QString CreateRobotConfigDialog::name()
{
    return ui->name->text();
}

QString CreateRobotConfigDialog::factory()
{
    return ui->factory->text();
}

QString CreateRobotConfigDialog::description()
{
    return ui->description->text();
}

QString CreateRobotConfigDialog::png()
{
    return ui->png->text();
}

QString CreateRobotConfigDialog::author()
{
    return ui->author->text();
}

bool CreateRobotConfigDialog::dhEnable()
{
    return ui->dh_enable_checkBox->isChecked();
}

bool CreateRobotConfigDialog::externAxisEnable()
{
    return ui->externAxis_checkBox->isChecked();
}

bool CreateRobotConfigDialog::dhParamEnable()
{
    return ui->dh_param_checkbox->isChecked();
}

void CreateRobotConfigDialog::on_add_clicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(nullptr, QObject::tr("打开机器人模型"),
                                                          QDir::currentPath(), QObject::tr("Step Files (*.step)"));
    step_model->setStringList(filePaths);
    ui->tabWidget->setCurrentIndex(0);

}


void CreateRobotConfigDialog::on_remove_clicked()
{
    step_model->setStringList(QStringList());
}


void CreateRobotConfigDialog::on_setpng_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, QObject::tr("打开机器人图片"),
                                                    QDir::currentPath(), QObject::tr("Png Files (*.png)"));
    ui->png->setText(filePath);
}


void CreateRobotConfigDialog::on_delpng_clicked()
{
    ui->png->setText("");
}


void CreateRobotConfigDialog::on_joint_number_textChanged(const QString &arg1)
{
    int num = arg1.toInt();
    qDebug()<<"num"<<num;
    int last_num = joint_layout->count();
    if(num > last_num){
        //< 数量变多
        for(int i = last_num;i<num;i++){
            JointConfigItem* joint_config_item = new JointConfigItem(this);
            JointInfo joint_info;
            joint_info.axis = i;
            joint_config_item->init(i+1,joint_info);
            joint_layout->addWidget(joint_config_item);
        }
    }
    else{
        //< 数量变少
        QVector<QLayoutItem*> last_widget;
        for(int i = num;i<last_num;i++){
            last_widget.append(joint_layout->itemAt(i));
        }
        for(int i = num;i<last_num;i++){
            joint_layout->removeItem(last_widget.at(i-num));
            delete last_widget.at(i-num)->widget();
        }
    }
    ui->tabWidget->setCurrentIndex(1);
}


void CreateRobotConfigDialog::on_edit_enable_clicked()
{
    if(ui->scrollAreaWidgetContents->isEnabled()){
        ui->scrollAreaWidgetContents->setEnabled(false);
        ui->model_tab->setEnabled(false);
        ui->joint_tab->setEnabled(false);
        ui->lua_tab->setEnabled(false);
    }
    else{
        ui->gridLayout->setEnabled(true);
        ui->scrollAreaWidgetContents->setEnabled(true);
        ui->horizontalLayout_2->setEnabled(true);
        ui->model_tab->setEnabled(true);
        ui->joint_tab->setEnabled(true);
        ui->lua_tab->setEnabled(true);
    }
}


void CreateRobotConfigDialog::on_dh_enable_checkBox_clicked()
{
     ui->tabWidget->setCurrentIndex(1);
}


void CreateRobotConfigDialog::on_test_lua_clicked()
{
    LuaThread* lua = new LuaThread();
    QString error = lua->startLua(ui->luatextEdit->toPlainText());
    if(error.size() > 0){
        ui->test_lua_result->setText(error);
        return;
    }
    QString str = ui->test_lua_data->text();
    str = str.replace("，",",");
    QStringList str_list = str.split(",");
    QVector<double> data;
    QVector<double> data_out;
    for(int i = 0;i<str_list.size();i++){
        data.append(str_list.at(i).toDouble());
    }
    lua->updateJoints(data,&data_out);
    QString result = "";
    for(int i = 0;i<data_out.size();i++){
        result+=QString::number(data_out.at(i))+",";
    }
    if(result.size() > 0){
        result.chop(1);
    }
    ui->test_lua_result->setText(result);
}


void CreateRobotConfigDialog::on_pushButton_clicked()
{

}

