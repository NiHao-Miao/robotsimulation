#include "incentivetrajectorydialog.h"
#include "ui_incentivetrajectorydialog.h"
#include "robotconfig.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qdebug.h>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QtConcurrent>
#include "vendor/dync/intf/optimize/curveoptimizeintf.h"
#include "qprogressindicator.h"

IncentiveTrajectoryDialog::IncentiveTrajectoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IncentiveTrajectoryDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("机器人激励轨迹");

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 设置水平滚动条的显示策略
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 设置垂直滚动条的显示策略
    ui->scrollArea->setWidgetResizable(true); // 当内容大小发生变化时，自动调整滚动区域的大小

    joint_layout = new QVBoxLayout(ui->scrollAreaWidgetContents);

    int axis_num = RobotConfig::Instance()->jointNumber();

    joint_para.clear();
    for(int i = 0;i<axis_num;i++){
        QHBoxLayout* layout = new QHBoxLayout();
        QLabel* name = new QLabel();
        name->setText("J"+QString::number(i+1));
        layout->addWidget(name);
        QVector<QLineEdit*> param;
        for(int j = 0;j<11;j++){
            QLineEdit* input = new QLineEdit();
            layout->addWidget(input);
            param.append(input);
        }
        joint_para.append(param);
        joint_layout->addLayout(layout);
    }
}

IncentiveTrajectoryDialog::~IncentiveTrajectoryDialog()
{
    delete ui;
}

QString IncentiveTrajectoryDialog::toString() const
{
    QJsonObject obj;
    obj["name"] = RobotConfig::Instance()->name();
    obj["axis_num"] = (int)RobotConfig::Instance()->jointNumber();
    for(int i = 0;i<joint_para.size();i++){
        QVector<QLineEdit*> param = joint_para[i];
        QJsonArray array;
        for(int j = 0;j<param.size();j++){
            bool ok = false;
            double v = param[j]->text().toDouble(&ok);
            if(!ok){
                v = 0;
            }
            array.append(v);
        }
        obj["J"+QString::number(i+1)] = array;
    }
    QJsonDocument doc(obj);
    return QString(doc.toJson(QJsonDocument::Indented));
}

void IncentiveTrajectoryDialog::import(const QString &str, bool show_err)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(),&jsonError);//QByteArray转QJsonDocument
    if (jsonError.error != QJsonParseError::NoError){
        QString errdata = QString(jsonError.errorString().toUtf8().constData());
        if(show_err)
        QMessageBox::warning(nullptr,"文件格式错误",errdata);
        return;
    }
    QJsonObject obj = doc.object();//QJsonDocument转QJsonObject
    int num = 0;
    if(obj.contains("axis_num")){
        num = obj["axis_num"].toInt();
        if(num != RobotConfig::Instance()->jointNumber()){
            if(show_err)
            QMessageBox::warning(nullptr,"文件格式错误","轴数量不一致");
            return;
        }
    }
    else{
        if(show_err)
        QMessageBox::warning(nullptr,"文件格式错误","轴数量不一致");
        return;
    }

    for(int i = 0 ;i<num;i++){
        QString n = "J"+QString::number(i+1);
        if(obj.contains(n)){
            QJsonArray array = obj[n].toArray();
            if(array.size() == 11){
                for(int j = 0;j<array.size();j++){
                    joint_para[i][j]->setText(QString::number(array[j].toDouble()));
                }
            }
            else{
                if(show_err)
                QMessageBox::warning(nullptr,"文件格式错误","轴参数数量错误");
                return;
            }
        }
        else{
            if(show_err)
            QMessageBox::warning(nullptr,"文件格式错误","轴参数不存在");
            return;
        }
    }

}

void IncentiveTrajectoryDialog::on_exportButton_clicked()
{
    QString str = toString();
    QString fileName=QFileDialog::getSaveFileName(this,"保存文件",
                                                    QDir::homePath(),
                                                          "json(*.json)");
    if(fileName.isEmpty()){
        return;
    }
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    //textEdit中内容保存
    file.write(str.toUtf8());
    file.close();
    QMessageBox::information(nullptr,"通知","文件保存成功!");
}


void IncentiveTrajectoryDialog::on_pushButton_clicked()
{

    QProgressIndicator *pIndicator = new QProgressIndicator(this);
    QFuture<std::vector<double> > f1 =QtConcurrent::run(this,&IncentiveTrajectoryDialog::optimize);
    while (!f1.isFinished()) {
        qApp->processEvents();
    }
    delete pIndicator;
    vector<double> coff = f1.result();
    if(coff.size() != (RobotConfig::Instance()->jointNumber()*11)){
        QMessageBox::warning(nullptr,"错误","激励轨迹系数计算有误："+coff.size());
        return;
    }
    for(int i = 0;i<RobotConfig::Instance()->jointNumber();i++){
        for(int j = 0;j<11;j++){
            joint_para[i][j]->setText(QString::number(coff[i*11+j]));
        }
    }
    QMessageBox::information(nullptr,"成功","激励轨迹系数计算完成");
}

std::vector<double> IncentiveTrajectoryDialog::optimize()
{
    vector<double> coff;
    HcdynLib::CurveOptimizeInterface* curve_optimize = HcdynLib::CurveOptimizeInterface::Create();
    vector<HcdynLib::JointParam> param;
    for(int i = 0;i<RobotConfig::Instance()->jointNumber();i++){
        HcdynLib::JointParam p ;
        JointInfo info = RobotConfig::Instance()->getJointInfo(i);
        p.theta = info.dh.theta*PI/180;
        p.d = info.dh.d*0.001;
        if(i == 0){
            p.alpha = 0;
            p.a =0;
        }
        else{
            p.alpha = RobotConfig::Instance()->getJointInfo(i-1).dh.alpha*PI/180;
            p.a = RobotConfig::Instance()->getJointInfo(i-1).dh.a*0.001;
        }

        p.joint_type = /*info.type < 3?HcdynLib::jtTranslation:*/HcdynLib::jtRotation;
        if(i == 0){
            p.joint_flag = HcdynLib::jfR3;
        }
        else{
            p.joint_flag = HcdynLib::jfR1;
        }
        p.pos_p = info.p_limit*PI/180;      // 关节位置正极限
        p.pos_n = info.n_limit*PI/180;      // 关节位置负极限
        p.max_vel = info.max_speed*PI/30/100;   // 关节最大速度
        p.max_acc = 0.2;                    // 关节最大加速度
        param.push_back(p);
    }
    curve_optimize->SetIncCurveParam(param);
    curve_optimize->Genetic();
    curve_optimize->GetOptimizeRet(coff);
    return coff;
}


void IncentiveTrajectoryDialog::on_importButton_clicked()
{
    QString fileName=QFileDialog::getOpenFileName(this,"打开激励轨迹文件",
                                                    QDir::homePath(),
                                                    "json(*.json)");
    if(fileName.isEmpty()){
        return;
    }
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray data =file.readAll();
    file.close();
    QJsonObject obj;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error != QJsonParseError::NoError){
        QMessageBox::information(nullptr,"错误","激励轨迹系数导入失败!文件格式错误:\n"+error.errorString());
        return;
    }
    obj = doc.object();

    if(obj.contains("axis_num")){
        int axis_num = obj["axis_num"].toInt();
        if(axis_num != RobotConfig::Instance()->jointNumber()){
            QMessageBox::information(nullptr,"错误","文件轴数不相等!\n实际轴数:"+QString::number(axis_num));
            return;
        }
    }
    else{
        QMessageBox::information(nullptr,"错误","缺少轴数属性!\n");
        return;
    }
    int axis_num = RobotConfig::Instance()->jointNumber();
    for(int i= 0;i<axis_num;i++){
        if(obj.contains("J"+QString::number(i+1))){
            QJsonArray array = obj["J"+QString::number(i+1)].toArray();
            if(array.size() != 11){
                QMessageBox::information(nullptr,"错误","轴"+QString::number(i+1)+"系数个数异常!\n实际数量:"+QString::number(array.size()));
                return;
            }
            for(int j =0;j<array.size();j++){
                joint_para[i][j]->setText(QString::number(array[j].toDouble()));
            }
        }
        else{
            QMessageBox::information(nullptr,"错误","缺少轴"+QString::number(i+1)+"系数!\n");
            return;
        }
    }

    QMessageBox::information(nullptr,"通知","激励轨迹系数导入成功!");
}

