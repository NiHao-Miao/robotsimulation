#ifndef SYSTEMSETTING_H
#define SYSTEMSETTING_H

#include <QObject>
#include <QSettings>

class SystemSetting:public QObject
{
    Q_OBJECT
public:
    SystemSetting(QObject * parent = nullptr);
    ~SystemSetting();

    static SystemSetting* Instance();

    QVariant getCustomSetting(QString key,QVariant defaultValue = QVariant()) const;

    void setCustomSetting(QString key,QVariant value);

    void reset();

public:
    quint32 getNetworkPort() const;
    void setNetworkPort(quint32 port);

    QString getNetworkComFormat() const;
    void setNetworkComFormat(const QString& data);

    bool getLastNetworkState() const;
    void setLastNetworkState(bool state);

    bool getOpenLastProject() const;
    void setOpenLastProject(bool state);

    quint32 getViewRefreshCycle() const;
    void setViewRefreshCycle(quint32 ms);

    bool getUdpNetEnable() const;
    void setUdpNetEnable(bool en);

    QString getTcpNetIpAddr() const;
    void setTcpNetIpAddr(const QString& ip_addr);

    quint32 getTcpNetPort() const;
    void setTcpNetIpPort( quint32 port);

    quint32 getMaxPointNum() const;
    void setMaxPointNum(quint32 count);

    bool getDebugLogEnable() const;
    void setDebugLogEnable(bool en);

    int getRobotCommandJoint() const;
    void setRobotCommandJoint(int en);

    bool  getRobotCommandWorld() const;

    quint32 getTrajectorEnableBindMValue() const;
    void setTrajectorEnableBindMValue(quint32 value);

    quint32 getTrajectorRedBindMValue() const;
    void setTrajectorRedBindMValue(quint32 value);

    quint32 getTrajectorGreenBindMValue() const;
    void setTrajectorGreenBindMValue(quint32 value);

    quint32 getTrajectorBlueBindMValue() const;
    void setTrajectorBlueBindMValue(quint32 value);

    quint32 getTrajectorYellowBindMValue() const;
    void setTrajectorYellowBindMValue(quint32 value);

    bool getNetworkAutoConnect() const;
    void setNetworkAutoConnect(bool en);

    quint32 getNetworkAutoConnectTime() const;
    void setNetworkAutoConnectTime(quint32 value);

signals:
    void networkPortChange();
    void networkComFormatChange();

private:
    void init();

private:
    QSettings* setting;
    bool is_init;

private:
    uint32_t network_port_;
    QString network_com_format;
    bool last_network_state;
    quint32 view_refresh_cycle;

};

#endif // SYSTEMSETTING_H
