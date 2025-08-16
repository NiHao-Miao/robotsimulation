#include "mqttmanage.h"
#include "qdebug.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostInfo>
#include <QDateTime>

const static QString mqtt_server_url = "43.138.136.232";
const static quint16 port = 1883;
const static QString username = "pangshubo";
const static QString password = "psb123456789.";

MqttManage::MqttManage(QObject *parent) : QObject(parent)
{
    mqttClient = nullptr;
}

MqttManage::~MqttManage()
{
    this->offline();
    delete mqttClient;
}

void MqttManage::init()
{
    QString clientID;
    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    clientID = "SZHC-"+QHostInfo::localHostName()+"-"+QString::number(qrand()); //合成 mqtt id
    //mqtt 初始化
    mqttClient = new QMQTT::Client(QHostAddress(mqtt_server_url), port);
    mqttClient->setClientId(clientID); //ClientId 不能重复，不然会覆盖之前连接的人的
    mqttClient->setUsername(username); //服务器账号密码，不需要则不用
    mqttClient->setPassword(password.toLatin1());
    mqttClient->setAutoReconnect(true);
    connect(mqttClient,&QMQTT::Client::error,this,[](const QMQTT::ClientError error){
        qDebug()<<"mqttClient error"<<error;
    });
    connect(mqttClient, &QMQTT::Client::connected, this, [this](){
        qDebug()<<"mqttClient connected !!!";
        this->mqttInit();
    });
    connect(mqttClient, &QMQTT::Client::subscribed, this, [](const QString& topic, const quint8 qos = 0){

    });
    connect(mqttClient, &QMQTT::Client::received, this, [this](const QMQTT::Message& message){
        QString topic = message.topic();
        QString text = message.payload();
        qDebug()<<"received"<<topic<<text;
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(),&jsonError);//QByteArray转QJsonDocument
        if (jsonError.error != QJsonParseError::NoError){
            QString errdata = QString(jsonError.errorString().toUtf8().constData());
            qDebug()<<errdata;
            return;
        }
        QJsonObject obj = doc.object();//QJsonDocument转QJsonObject
        if(topic == "online"){
            emit this->onOnline(obj["name"].toString());
        }
        else if(topic == "message"){
            QString send = obj["send"].toString();
            QString to = obj["to"].toString();
            QString text = obj["text"].toString();
        }
        else if(topic == ( QHostInfo::localHostName()+"_message")){
            QString send = obj["send"].toString();
            QString to = obj["to"].toString();
            QString text = obj["text"].toString();
        }
    });
    mqttClient->connectToHost();
}

MqttManage *MqttManage::Instance()
{
    static MqttManage mqtt_manage;
    return &mqtt_manage;
}

void MqttManage::mqttInit()
{
    //< 上线通知
    this->online();

    //< 订阅主题
    QStringList subscribe_list;
    subscribe_list.push_back("online");
    subscribe_list.push_back("message");
    subscribe_list.push_back(QHostInfo::localHostName()+"_message");
    for(int i = 0 ;i<subscribe_list.size();i++){
        mqttClient->subscribe(subscribe_list[i]);
    }
}

void MqttManage::online()
{
    QJsonObject obj;
    obj["name"]=QHostInfo::localHostName();
    obj["time"]=QDateTime::currentDateTime().toString(Qt::LocalDate);
    send("online",obj,true);
}

void MqttManage::offline()
{
    QJsonObject obj;
    obj["name"]=QHostInfo::localHostName();
    obj["time"]=QDateTime::currentDateTime().toString(Qt::LocalDate);
    send("offline",obj);
}

void MqttManage::send(const QString &topic, const QJsonObject &obj,bool retain)
{
    if(mqttClient == nullptr)
        return;
    QMQTT::Message mes;
    mes.setTopic(topic);
    mes.setDup(retain);

    QJsonDocument doc(obj);
    QString text = QString(doc.toJson(QJsonDocument::Indented));
    mes.setPayload(text.toLatin1());
    mqttClient->publish(mes);
}
