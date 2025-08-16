#include "occQt.h"
#include "autoupdate.h"
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QtConcurrent>
#include <QSplashScreen>
#include <QPixmap>
#include <QHostInfo>
#include <QTime>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include "mqttmanage.h"
#include "icutils.h"
#include "systemsetting.h"

//<  原文链接：https://blog.csdn.net/Albert_weiku/article/details/127652571

QString className("szhc");                           // 自定义的类别
QString ext(".hctobot");                                       // 关联的文件类型
QString extDes("华成机器人仿真平台工程文件");                              // 该文件类型描述

void setDefaultProgram(const QString &extension, const QString &programPath,const QString& iconPath) {

#if 0
    QSettings reg("HKEY_CURRENT_USER\\Software\\Classes\\.", QSettings::NativeFormat);
    // 设置文件关联
    reg.setValue(extension + "/", "");
    reg.setValue(extension + "/DefaultIcon/", programPath + ",0");
    reg.setValue(extension + "/shell/open/command/", "\"" + programPath + "\" \"%1\"");
    qDebug()<<"setDefaultProgram extension"<<extension<<"programPath"<<programPath;
#endif

    QString baseUrl("HKEY_CURRENT_USER\\Software\\Classes");    // 要添加的顶层目录
    QSettings settingClasses(baseUrl, QSettings::NativeFormat);
    settingClasses.setValue("/" + className + "/Shell/Open/Command/.", "\"" + programPath + "\" \"%1\"");
    settingClasses.setValue("/" + className + "/.", extDes);

    //这两种方式二选一,或者全配置也没关系
    settingClasses.setValue("/" + className + "/DefaultIcon/.", programPath + ",0");//使用这个程序后的默认图标
    settingClasses.setValue("/" + ext + "/DefaultIcon/.",iconPath + ",0");// 这个ext文件类型的关联图标

    // 关联ext 和 类别
    settingClasses.setValue("/" + ext + "/OpenWithProgIds/" + className, "");

    // 立即保存该修改
    settingClasses.sync();

    ////调用 SHChangeNotify 函数以通知 Shell 更新其图标缓存。
//    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QFile file("debug.log");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    QString timeStamp = QDateTime::currentDateTime().toString();
    QString typeString;
    switch (type) {
    case QtDebugMsg: typeString = "Debug"; break;
    case QtInfoMsg: typeString = "Info"; break;
    case QtWarningMsg: typeString = "Warning"; break;
    case QtCriticalMsg: typeString = "Critical"; break;
    case QtFatalMsg: typeString = "Fatal"; break;
    }
    QString context_info = QString("File:(%1) Line:(%2) Func:(%3)")
                               .arg(QString(context.file)).arg(context.line)
                               .arg(QString(context.function));
    textStream << timeStamp.toUtf8() << ":" << typeString.toUtf8() << ":" << context_info.toUtf8() << ":" << msg.toUtf8() << "\n";
    file.flush(); // 确保写入文件，而不是等到文件被关闭时才写。
    file.close(); // 关闭文件（可选，通常在程序结束时自动关闭）
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(true); // 设置在最后一个窗口关闭时退出

    if(SystemSetting::Instance()->getDebugLogEnable()){
        // 安装自定义消息处理程序
        qInstallMessageHandler(myMessageOutput);
    }
    qApp->setStyleSheet(":/stylesheet/default.qss");
    // 创建一个SplashScreen对象
    QSplashScreen splash(QPixmap("://Resources/logo.png"));
    splash.show();

    QApplication* app = qobject_cast<QApplication*>(QCoreApplication::instance());
    QString appName = app->applicationName()+".exe";
    qDebug()<<"appName"<<appName;
    QFileInfo myapp(appName);
    QString programPath = myapp.absoluteFilePath().replace("/","\\\\"); // 程序路径
    QFileInfo ico("Resources/logo.ico");
    QString iconPath = ico.absoluteFilePath();

    setDefaultProgram(ext, programPath,iconPath);
    splash.showMessage("加载UI...",Qt::AlignHCenter);
    occQt w;
    w.show();
    splash.showMessage("加载默认机器人模型...",Qt::AlignHCenter);
    w.initDefaultRobotModelList();
    splash.showMessage("加载用户机器人模型...",Qt::AlignHCenter);
    w.initRobotModelList();
    splash.showMessage("检查更新...",Qt::AlignHCenter);
    AutoUpdate::Instance()->checkUpdate(false);

    splash.hide();
    splash.showMessage("加载上一次的模型...",Qt::AlignHCenter);
    w.restoreLastStatus();
    splash.showMessage("启动完成",Qt::AlignHCenter);

    return a.exec();
}
