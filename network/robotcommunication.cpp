#include "robotcommunication.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "qdebug.h"
#include "icutils.h"
#include "systemsetting.h"
#include <QColor>
#include <QNetworkProxy>

RobotCommunication::RobotCommunication(QObject *parent) : QThread(parent),udpSocket(nullptr),tcpclient(nullptr)
{
    initTcp();
    QAbstractSocket::connect(&udp_timer,&QTimer::timeout,this,[this](){
        isConnect = false;
    });
    udp_timer.setSingleShot(false);
}

void RobotCommunication::startThread()
{
    is_run = true;
//    this->start();
    tcp_timer.start(SystemSetting::Instance()->getViewRefreshCycle());
}

void RobotCommunication::stopThread()
{
    is_run = false;
    tcp_timer.stop();
}

void RobotCommunication::run()
{
    while (is_run) {
        this->sendHostData();
        if(!isConnect){
            is_run = false;
        }
        msleep(20);
    }
}

void RobotCommunication::initUdp(int port ,const QString& format)
{
    tcp_timer.stop();
    if(udpSocket){
        closeUdp();
    }
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, port);
    udpSocket->setProxy(QNetworkProxy::NoProxy);

    QAbstractSocket::connect(udpSocket, &QUdpSocket::readyRead,
            this, &RobotCommunication::readPendingDatagrams);

    default_format = format;
    if(format == "")
        default_format = "[<J1>,<J2>,<J3>,<J4>,<J5>,<J6>]";
    QRegularExpression regex("<J(\\d+)>");
    QRegularExpressionMatchIterator matchIterator = regex.globalMatch(default_format);
    uint32_t i = 0;
    joint_id_map.clear();
    max_joint_id = 0;
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        quint32 id  = match.captured(1).toUInt();
        joint_id_map.insert(i,id);
        if(id > max_joint_id){
            max_joint_id = id;
        }
        ++i;
    }
    qDebug() << joint_id_map;
    QString pattern = "<J\\d+>";
    QString replacement = "%f";
    QRegularExpression re(pattern);
    default_format.replace(re, replacement);

    qDebug() << default_format;
    udp_timer.start(100);
}

void RobotCommunication::closeUdp()
{
    if(udpSocket){
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;
    }
}

void RobotCommunication::initTcp()
{
    tcpclient = new QTcpSocket();
    isConnect = false;
    tcpclient->setProxy(QNetworkProxy::NoProxy);
    QObject::connect(tcpclient, &QTcpSocket::readyRead, this,&RobotCommunication::tcpclientreaddata);
    QObject::connect(tcpclient, &QTcpSocket::connected, this,&RobotCommunication::tcpclientconnect);
    QObject::connect(tcpclient, &QTcpSocket::disconnected, this,&RobotCommunication::tcpclientdisconnect);
    QAbstractSocket::connect(&tcp_timer,&QTimer::timeout,this,[this](){
        this->sendHostData();
    });
    udp_timer.stop();
}


bool RobotCommunication::connect(const QString &ip, quint32 port)
{
    if(isconnect()){
        disconnect();
    }
    if(ip.length()){
        ip_addr = ip;
        ip_port = port;
    }
    qDebug()<<"connect"<<ip_addr<<ip_port;
    tcpclient->connectToHost(ip_addr,ip_port);
    bool ret = tcpclient->waitForConnected(100);
    if(!ret){
        qDebug()<<"error"<<tcpclient->error()<<tcpclient->errorString()<<"state"<<tcpclient->state();
    }
    return ret;
}

bool RobotCommunication::isconnect()
{
    return isConnect;
}

void RobotCommunication::disconnect()
{
    tcpclient->close();
    isConnect = false;
    emit connectStatusChanged(false);
}

void RobotCommunication::sendHostData()
{
    QString data;
    switch (SystemSetting::Instance()->getRobotCommandJoint()) {
    case 0:
        data = "{\"dsID\":\"www.hc-system.com.RemoteMonitor\",\"reqType\":\"query\",\"queryAddr\":[\"faxis-0\",\"faxis-1\",\"faxis-2\",\"faxis-3\",\"faxis-4\",\"faxis-5\",\"faxis-6\",\"faxis-7\",\"M-0\",\"M-1\"]}";
        break;
    case 2:
        data = "{\"dsID\":\"www.hc-system.com.RemoteMonitor\",\"reqType\":\"query\",\"queryAddr\":[\"world-0\",\"world-1\",\"world-2\",\"world-3\",\"world-4\",\"world-5\",\"world-6\",\"world-7\",\"M-0\",\"M-1\"]}";
        break;
    case 1:
    default:
        data = "{\"dsID\":\"www.hc-system.com.RemoteMonitor\",\"reqType\":\"query\",\"queryAddr\":[\"axis-0\",\"axis-1\",\"axis-2\",\"axis-3\",\"axis-4\",\"axis-5\",\"axis-6\",\"axis-7\",\"M-0\",\"M-1\"]}";
        break;
    }

    this->sendData(data.toUtf8());
}

int RobotCommunication::sendData(const QByteArray &data)
{
    int32_t ret=  tcpclient->write(data);
    tcpclient->flush();
    if(ret < 0){
        isConnect = false;
    }
//    qDebug()<<"sendData"<<data;
    return ret;
}

void RobotCommunication::tcpclientreaddata() {
    quint32 read_len = tcpclient->bytesAvailable();
    QByteArray read_data = tcpclient->readAll();
//    qDebug()<<"read_data"<<read_data;

    // 将JSON字符串转换为QJsonDocument对象
    QJsonDocument jsonDoc = QJsonDocument::fromJson(read_data);
    QVector<double> joints;
    QVector<quint32> m_value;
    // 检查JSON是否有效
    if (!jsonDoc.isNull()) {
        // 将QJsonDocument转换为QJsonObject
        QJsonObject jsonObj = jsonDoc.object();

        // 获取queryData字段的值
        QJsonValue queryDataValue = jsonObj.value("queryData");

        // 检查queryData字段的值是否为数组类型
        if (queryDataValue.isArray()) {
            // 将queryData字段的值转换为QJsonArray
            QJsonArray queryDataArray = queryDataValue.toArray();

            // 遍历QJsonArray并打印元素
            foreach (const QJsonValue& value, queryDataArray) {
                if(joints.size() == 8){
                    m_value.append(value.toString().toUInt());
                }
                else{
                    joints.append(value.toString().toDouble());
                }
            }
        }
    }
    if(SystemSetting::Instance()->getRobotCommandWorld()){
        emit updateRobotWorld(joints);
    }
    else{
        emit updateRobotJoints(joints);
    }
    emit updateRobotMValue(m_value);
    if(SystemSetting::Instance()->getTrajectorEnableBindMValue()){
        quint32 index = SystemSetting::Instance()->getTrajectorEnableBindMValue() - 1;
        emit trajectorEnableChange(m_value[index/32] & (1<<(index%32)));
    }
    if(SystemSetting::Instance()->getTrajectorRedBindMValue()){
        quint32 index = SystemSetting::Instance()->getTrajectorRedBindMValue() - 1;
        if(m_value[index/32] & (1<<(index%32))){
            emit trajectorColorChange(QColor("red"));
        }
    }
    if(SystemSetting::Instance()->getTrajectorYellowBindMValue()){
        quint32 index = SystemSetting::Instance()->getTrajectorYellowBindMValue() - 1;
        if(m_value[index/32] & (1<<(index%32))){
            emit trajectorColorChange(QColor("yellow"));
        }
    }
    if(SystemSetting::Instance()->getTrajectorBlueBindMValue()){
        quint32 index = SystemSetting::Instance()->getTrajectorBlueBindMValue() - 1;
        if(m_value[index/32] & (1<<(index%32))){
            emit trajectorColorChange(QColor("blue"));
        }
    }
    if(SystemSetting::Instance()->getTrajectorGreenBindMValue()){
        quint32 index = SystemSetting::Instance()->getTrajectorGreenBindMValue() - 1;
        if(m_value[index/32] & (1<<(index%32))){
            emit trajectorColorChange(QColor("green"));
        }
    }

}

void RobotCommunication::tcpclientconnect()
{
    isConnect = true;
    emit connectStatusChanged(true);
}

void RobotCommunication::tcpclientdisconnect()
{
    isConnect = false;
    emit connectStatusChanged(false);
}

void RobotCommunication::tcpclientstateChanged(QAbstractSocket::SocketState)
{

}

void RobotCommunication::tcpclienterror(QAbstractSocket::SocketError)
{

}

void RobotCommunication::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QString data = datagram.data();
        QVariantList values;
        QVector<double> joints;
        joints.resize(max_joint_id);
        bool ret = ICUtils::stringFormatInput(data,default_format,&values);
        if(ret){
            for(int i = 0;i<values.size();i++){
                quint32 j = joint_id_map.value(i)-1;
                switch (values.at(i).type()) {
                case QVariant::Int:
                    joints[j]=values.at(i).toInt();
                    break;
                case QVariant::Double:
                    joints[j]=values.at(i).toDouble();
                    break;
                }
            }
            emit updateRobotJoints(joints);
        }
        else{
            qDebug()<<"网络通讯格式错误:"<<default_format<<data;
        }
    }
    emit connectStatusChanged(true);
}
