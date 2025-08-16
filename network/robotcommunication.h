#ifndef ROBOTCOMMUNICATION_H
#define ROBOTCOMMUNICATION_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QTcpSocket>
#include <QTimer>

class RobotCommunication : public QThread
{
    Q_OBJECT
public:
    explicit RobotCommunication(QObject *parent = nullptr);

    void initUdp(int port ,const QString& format);
    void closeUdp();

    void startThread();
    void stopThread();

    virtual void run();

    ///
    /// \brief connect  TCP 连接
    /// \param ip
    /// \param port
    /// \return
    ///
    bool connect(const QString& ip,quint32 port);
    bool isconnect();
    void disconnect();

    void sendHostData();

    int sendData(const QByteArray& data);

    void setAutoConnect(bool en);

signals:
    void tcpRecvData(QByteArray );
    void updateRobotJoints(QVector<double>);
    void updateRobotWorld(QVector<double>);
    void updateRobotMValue(QVector<quint32>);
    void trajectorEnableChange(bool);
    void trajectorColorChange(const QColor&);
    void recvData(const QString& data);

    void connectStatusChanged(bool status);

public slots:
    void tcpclientreaddata();
    void tcpclientconnect();
    void tcpclientdisconnect();
    void tcpclientstateChanged(QTcpSocket::SocketState);
    void tcpclienterror(QAbstractSocket::SocketError);
    void readPendingDatagrams();

private:
    void initTcp();

private:
    QUdpSocket* udpSocket;
    bool is_run;

    QString ip_addr;
    quint16 ip_port;
    bool isConnect;
    QTcpSocket* tcpclient;
    QTimer tcp_timer;
    QTimer udp_timer;

private:
    QString default_format;
    quint32 max_joint_id;
    QMap<quint32,quint32> joint_id_map;     //< 坐标映射
};

#endif // ROBOTCOMMUNICATION_H
