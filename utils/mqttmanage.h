#ifndef MQTTMANAGE_H
#define MQTTMANAGE_H

#include <QObject>
#include "qmqtt.h"
#include "qmqtt_client.h"
#include <QJsonObject>

class MqttManage : public QObject
{
    Q_OBJECT
public:
    explicit MqttManage(QObject *parent = nullptr);
    ~MqttManage();

    void init();

    static MqttManage* Instance();
signals:
    void onOnline(const QString&);

private:
    void mqttInit();
    void online();
    void offline();

    void send(const QString& topic, const QJsonObject& obj, bool retain = false);

private:
    QMQTT::Client *mqttClient;
};

#endif // MQTTMANAGE_H
