#include "robotconfig.h"
#include <QDir>
#include "createrobotconfigdialog.h"
#include "qdebug.h"
#include "icutils.h"
#include <QTextCodec>

RobotConfig::RobotConfig(QObject *parent) : QObject(parent),setting(nullptr),L(nullptr)
{

}

RobotConfig::~RobotConfig()
{
    if(setting){
        setting->sync();
        delete setting;
    }
    if(L){
        lua_close(L);
        L = nullptr;
    }
}


QString RobotConfig::robotAbsPath()
{
    return this->path;
}

bool RobotConfig::isLoad() const
{
    return setting != nullptr;
}

bool RobotConfig::loadRobotConfig(const QString &path)
{
    if(setting != nullptr){
        delete setting;
    }
    setting = new QSettings(path, QSettings::IniFormat);
    setting->setIniCodec(QTextCodec::codecForName("utf-8"));
    QFileInfo file(path);
    this->path = file.absolutePath();
    bool permission = file.permission(QFile::ReadOwner|QFile::ReadUser|QFile::ReadGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteGroup|QFile::ExeOwner|QFile::ExeUser|QFile::ExeGroup);
    qDebug()<<"permission"<<permission<<file.isWritable()<<setting->status();
    return QSettings::NoError == setting->status();
}

void RobotConfig::loadRobotConfig(CreateRobotConfigDialog* dialog){
    bool ret = this->loadRobotConfig(ICUtils::modelNameToPath(dialog->name()));
    if(!ret){
        qDebug()<<"参数更新失败"<<ICUtils::modelNameToPath(dialog->name());
        return;
    }
    this->setConfig(dialog->name(),dialog->factory(),dialog->description(),dialog->png(),dialog->author(),dialog->relStepFiles());
    this->setDhEnable(dialog->dhEnable());
    this->setJointNumber(dialog->jointNumber());
    this->setDhParamEnable(dialog->dhParamEnable());
    this->setExternAxis(dialog->externAxisEnable());
    auto infos = dialog->jointsInfo();
    for(int i= 0;i<infos.size();i++){
        this->setJointInfo(i,infos[i]);
    }
    setLuaScript(dialog->luaScript());
    qDebug()<<"模型配置更新"<<dialog->name();
}

DH RobotConfig::getDH(quint16 id) const
{
    DH dh;
    setting->beginGroup(QString("joint_%1").arg(id));
    dh.alpha = setting->value("alpha",0).toFloat();
    dh.a = setting->value("a",0).toFloat();
    dh.d = setting->value("d",0).toFloat();
    dh.theta = setting->value("theta",0).toFloat();
    setting->endGroup();
    return dh;
}

void RobotConfig::setDH(int id, const DH &dh)
{
    setting->beginGroup(QString("joint_%1").arg(id));
    setting->setValue("alpha",dh.alpha);
    setting->setValue("a",dh.a);
    setting->setValue("d",dh.d);
    setting->setValue("theta",dh.theta);
    setting->endGroup();
}

QStringList RobotConfig::absStepFiles() const
{
    QDir dir(path);
    dir.cd(this->name());
    QStringList abs_path;
    QStringList file_path = this->stepFiles();
    for(int i = 0,len = file_path.size();i<len;i++){
        abs_path.append(dir.absoluteFilePath(file_path.at(i)));
    }
    return abs_path;
}

void RobotConfig::setConfig(const QString &name, const QString &factory, const QString &description, const QString &png, const QString &author, const QStringList &stepFiles)
{
    setCustomSetting("name",name);
    setCustomSetting("factory",factory);
    setCustomSetting("description",description);
    setCustomSetting("png",png);
    setCustomSetting("author",author);
    setCustomSetting("stepFiles",stepFiles);
}

QString RobotConfig::getLuaScript() const
{
    return getCustomSetting("lua_script","-- 请在内部完善函数体，不要修改函数名和参数，需要返回一个浮点数组\n\n"
                                        "function hc_updateJoints(joints_in) \n"
                                        "-- 请在此处写 \n"
                                        "-- 定义一个数组,输出关节坐标 \n"
                                        "local joints_out = {} \n"
                                        " \n"
                                        "-- 给数组赋值 \n"
                                        "joints_out = joints_in \n"
                                        "joints_out[2] = joints_in[2] \n"
                                        "joints_out[3] = joints_in[3] \n"
                                        "joints_out[4] = joints_in[4] \n"
                                        "joints_out[5] = joints_in[5] \n"
                                        "joints_out[6] = joints_in[6] \n"
                                        " \n"
                                        "return joints_out \n"
                                        "end").toString();
}

void RobotConfig::setLuaScript(const QString &data)
{
    setCustomSetting("lua_script",data);
}



QVariant RobotConfig::getCustomSetting(QString key,QVariant defaultValue) const{
    QString name = QString("custom/%1").arg(key);
    return setting->value(name,defaultValue);
}

void RobotConfig::setCustomSetting(QString key, QVariant value){
    QString name = QString("custom/%1").arg(key);
    setting->setValue(name,value);
}

JointInfo RobotConfig::getJointInfo(quint16 id) const
{
    JointInfo info;
    setting->beginGroup(QString("joint_%1").arg(id));
    info.axis = setting->value("axis",0).toUInt();
    info.type = setting->value("type",0).toUInt();
    info.dir = setting->value("dir",-1).toInt();
    info.x = setting->value("x",0).toFloat();
    info.y = setting->value("y",0).toFloat();
    info.z = setting->value("z",0).toFloat();
    info.n_limit = setting->value("n_limit",-360).toFloat();
    info.p_limit = setting->value("p_limit",360).toFloat();
    info.default_origin = setting->value("default_origin",0).toFloat();
    info.dh.alpha = setting->value("alpha",0).toFloat();
    info.dh.a = setting->value("a",0).toFloat();
    info.dh.d = setting->value("d",0).toFloat();
    info.dh.theta = setting->value("theta",0).toFloat();
    info.max_speed = setting->value("max_speed",3000).toFloat();
    setting->endGroup();
    return info;
}

void RobotConfig::setJointInfo(int id, const JointInfo &info)
{
    setting->beginGroup(QString("joint_%1").arg(id));
    setting->setValue("axis",info.axis);
    setting->setValue("type",info.type);
    setting->setValue("dir",info.dir);
    setting->setValue("x",info.x);
    setting->setValue("y",info.y);
    setting->setValue("z",info.z );
    setting->setValue("n_limit",info.n_limit);
    setting->setValue("p_limit",info.p_limit);
    setting->setValue("default_origin",info.default_origin);
    setting->setValue("alpha",info.dh.alpha);
    setting->setValue("a",info.dh.a);
    setting->setValue("d",info.dh.d);
    setting->setValue("theta",info.dh.theta);
    setting->setValue("max_speed",info.max_speed);
    setting->endGroup();
}
