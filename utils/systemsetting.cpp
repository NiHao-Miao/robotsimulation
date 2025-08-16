#include "systemsetting.h"
#include "qdebug.h"
#include <QTextCodec>

SystemSetting::SystemSetting(QObject *parent):QObject(parent)
{
    setting = new QSettings("./config/sys.ini", QSettings::IniFormat);
    setting->setIniCodec(QTextCodec::codecForName("utf-8"));
    is_init  = false;
    init();
}

SystemSetting::~SystemSetting()
{

}

SystemSetting *SystemSetting::Instance()
{
    static SystemSetting system_setting;
    return &system_setting;
}


QVariant SystemSetting::getCustomSetting(QString key,QVariant defaultValue) const{
    QString name = QString("custom/%1").arg(key);
    return setting->value(name,defaultValue);
}

void SystemSetting::setCustomSetting(QString key, QVariant value){
    QString name = QString("custom/%1").arg(key);
    setting->setValue(name,value);
}

void SystemSetting::reset()
{
    setting->clear();
}

quint32 SystemSetting::getNetworkPort() const
{
    return network_port_;
}

void SystemSetting::setNetworkPort(quint32 port)
{
    SystemSetting::Instance()->setCustomSetting("network_port",port);
    network_port_ = port;
    emit networkPortChange();
}

QString SystemSetting::getNetworkComFormat() const
{
    return network_com_format;
}

void SystemSetting::setNetworkComFormat(const QString &data)
{
    SystemSetting::Instance()->setCustomSetting("network_com_format",data);
    network_com_format = data;
    emit networkComFormatChange();
}

bool SystemSetting::getLastNetworkState() const
{
    return last_network_state;
}

void SystemSetting::setLastNetworkState(bool state)
{
    setCustomSetting("last_network_state",state);
    last_network_state = state;
}

bool SystemSetting::getOpenLastProject() const
{
    return getCustomSetting("open_last_project",true).toBool();
}

void SystemSetting::setOpenLastProject(bool state)
{
    setCustomSetting("open_last_project",state);
}

quint32 SystemSetting::getViewRefreshCycle() const
{
    return view_refresh_cycle;
}

void SystemSetting::setViewRefreshCycle(quint32 ms)
{
    view_refresh_cycle = ms;
    setCustomSetting("view_refresh_cycle",ms);
}

bool SystemSetting::getUdpNetEnable() const
{
    return getCustomSetting("udp_net_enable",true).toBool();
}

void SystemSetting::setUdpNetEnable(bool en)
{
    setCustomSetting("udp_net_enable",en);
}

QString SystemSetting::getTcpNetIpAddr() const
{
    return getCustomSetting("tcp_net_ip_addr","192.168.4.4").toString();
}

void SystemSetting::setTcpNetIpAddr(const QString &ip_addr)
{
    setCustomSetting("tcp_net_ip_addr",ip_addr);
}

quint32 SystemSetting::getTcpNetPort() const
{
    return getCustomSetting("tcp_net_port","9760").toUInt();
}

void SystemSetting::setTcpNetIpPort(quint32 port)
{
    setCustomSetting("tcp_net_port",port);
}

quint32 SystemSetting::getMaxPointNum() const
{
    return getCustomSetting("max_point_num","10000").toUInt();
}

void SystemSetting::setMaxPointNum(quint32 count)
{
    setCustomSetting("max_point_num",count);
}

bool SystemSetting::getDebugLogEnable() const
{
    return getCustomSetting("debug_log",false).toBool();
}

void SystemSetting::setDebugLogEnable(bool en)
{
    setCustomSetting("debug_log",en);
}

int SystemSetting::getRobotCommandJoint() const
{
    bool ok= false;
    uint id = getCustomSetting("robot_command",1).toUInt(&ok);
    if(ok){
        return id;
    }
    else {
        return 1;
    }
}

void SystemSetting::setRobotCommandJoint(int en)
{
    setCustomSetting("robot_command",en);
}

bool SystemSetting::getRobotCommandWorld() const
{
    return getRobotCommandJoint() == 2;
}

quint32 SystemSetting::getTrajectorEnableBindMValue() const
{
    return getCustomSetting("trajector_enable_bind_m_value",0).toUInt();
}

void SystemSetting::setTrajectorEnableBindMValue(quint32 value)
{
    setCustomSetting("trajector_enable_bind_m_value",value);
}

quint32 SystemSetting::getTrajectorRedBindMValue() const
{
    return getCustomSetting("trajector_red_bind_m_value",0).toUInt();

}

void SystemSetting::setTrajectorRedBindMValue(quint32 value)
{
    setCustomSetting("trajector_red_bind_m_value",value);

}

quint32 SystemSetting::getTrajectorGreenBindMValue() const
{
    return getCustomSetting("trajector_green_bind_m_value",0).toUInt();

}

void SystemSetting::setTrajectorGreenBindMValue(quint32 value)
{
    setCustomSetting("trajector_green_bind_m_value",value);

}

quint32 SystemSetting::getTrajectorBlueBindMValue() const
{
    return getCustomSetting("trajector_blue_bind_m_value",0).toUInt();

}

void SystemSetting::setTrajectorBlueBindMValue(quint32 value)
{
    setCustomSetting("trajector_blue_bind_m_value",value);

}

quint32 SystemSetting::getTrajectorYellowBindMValue() const
{
    return getCustomSetting("trajector_yellow_bind_m_value",0).toUInt();

}

void SystemSetting::setTrajectorYellowBindMValue(quint32 value)
{
    setCustomSetting("trajector_yellow_bind_m_value",value);

}

bool SystemSetting::getNetworkAutoConnect() const
{
    return getCustomSetting("network_auto_connect",true).toBool();
}

void SystemSetting::setNetworkAutoConnect(bool en)
{
    setCustomSetting("network_auto_connect",en);
}

quint32 SystemSetting::getNetworkAutoConnectTime() const
{
    return getCustomSetting("network_auto_connect_time",5).toUInt();
}

void SystemSetting::setNetworkAutoConnectTime(quint32 value)
{
    setCustomSetting("network_auto_connect_time",value);
}

void SystemSetting::init()
{
    is_init = true;
    network_port_ = getCustomSetting("network_port",7755).toUInt();
    network_com_format = getCustomSetting("network_com_format","[<J1>,<J2>,<J3>,<J4>,<J5>,<J6>]").toString();
    last_network_state = getCustomSetting("last_network_state",true).toBool();
    view_refresh_cycle =  getCustomSetting("view_refresh_cycle",50).toUInt();
}
