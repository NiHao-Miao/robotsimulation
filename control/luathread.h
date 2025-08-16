#ifndef LUATHREAD_H
#define LUATHREAD_H

#include "lua.hpp"
#include <QByteArray>
#include <QFileInfo>
#include <QImage>
#include <QMap>
#include <QString>
#include <QThread>
#include <QTcpSocket>
#include <QTime>

class LuaThread : public QThread {
    Q_OBJECT
public:
    LuaThread(QObject* parent= nullptr);
    ~LuaThread();

public:
    friend class DeviceControl;
    friend void lstop(lua_State *L, lua_Debug *ar);


public:
    static QString formatHHMMSS(qint32 ms);

    QString startLua(const QString &lua_file);

    void initLuaFunction();

    void destroyLua();

    int runTime();

    bool isRunning();

    bool updateJoints(const QVector<double> & joints,QVector<double>* joints_out);

    ///
    /// \brief updateBase   更新基座位置
    /// \param joints
    /// \param joints_out
    /// \return
    ///
    bool updateBase(const QVector<double> & joints,QVector<double>* joints_out);

signals:
    void runFinish();

    void appendTestResult(const QString &test, bool result);

    void appendInfoText(const QString &text);

    void sendTcpText(const QString &text);

    void runError(const QString &info);


public:
    void run() override;
    QMap<QString, QString> function_list;
    QMap<QString, QString> function_help_list;

private:
    QFileInfo script_info;
    QTime exec_time;


private:
    std::mutex thread_lock;
    lua_State *L = nullptr;
};

#endif // LUATHREAD_H
