#ifndef ROBOTCONFIG_H
#define ROBOTCONFIG_H

#include <QObject>
#include <QSettings>
#include "gp_Trsf.hxx"
#include "lua.hpp"

class DH {
public:
    DH(){
        alpha = 0;
        a = 0;
        d =0;
        theta = 0;
    }
    double alpha;
    double a;
    double d;
    double theta;

    /// <summary>
    /// 返回变换矩阵
    /// </summary>
    /// <param name="theta"></param>
    /// <returns></returns>
    gp_Trsf toTrsf(double theta1 = 0);
};

class JointInfo{
public:
    JointInfo(){
        axis = 0;
        type = 1;
        dir = 1;
        n_limit = -360;
        p_limit = 360;
        max_speed = 3000;
        x = 0;
        y =0 ;
        z = 0;
        default_origin = 0.0;
    }
    uint32_t axis;
    uint32_t type;
    int32_t dir;
    double n_limit;  //< 负限位
    double p_limit;  //< 正限位
    double x;
    double y;
    double z;
    double default_origin;//< 默认零点
    DH dh;
    double max_speed;   //< 最大速度RPM
};

class CreateRobotConfigDialog;

class RobotConfig : public QObject
{
    Q_OBJECT
public:
    explicit RobotConfig(QObject *parent = nullptr);
    virtual ~RobotConfig();

    QString robotAbsPath();

    ///
    /// \brief isLoad  是否加载
    /// \return
    ///
    bool isLoad() const;

    ///
    /// \brief loadRobotConfig  加载机器人文件
    /// \param path
    /// \return
    ///
    bool loadRobotConfig(const QString& path);

    ///
    /// \brief loadRobotConfig  加载机器人文件
    /// \param path
    /// \return
    ///
    void loadRobotConfig(CreateRobotConfigDialog *dialog);


    ///
    /// \brief name  名字
    /// \return
    ///
    QString name() const{
        return getCustomSetting("name").toString();
    }
    ///
    /// \brief factory 厂家
    /// \return
    ///
    QString factory() const{
        return getCustomSetting("factory").toString();
    }
    ///
    /// \brief description  描述
    /// \return
    ///
    QString description() const{
        return getCustomSetting("description").toString();
    }

    ///
    /// \brief png   图片
    /// \return
    ///
    QString png() const{
        return getCustomSetting("png").toString();
    }

    ///
    /// \brief author  作者
    /// \return
    ///
    QString author() const{
        return getCustomSetting("author").toString();
    }

    ///
    /// \brief step  模型文件
    /// \return
    ///
    QStringList stepFiles() const{
        return getCustomSetting("stepFiles").toStringList();
    }

    ///
    /// \brief jointNumber  关节数
    /// \return
    ///
    quint32 jointNumber() const{
        return getCustomSetting("jointNumber",6).toUInt();
    }

    ///
    /// \brief setJointNumber  设置关节数
    /// \param num
    ///
    void setJointNumber(quint32 num) {
        setCustomSetting("jointNumber",num);
    }

    ///
    /// \brief getDhEnable  是否使用DH参数
    /// \return
    ///
    bool getDhEnable() const{
        return getCustomSetting("DhEnable",false).toBool();
    }

    ///
    /// \brief getDhEnable  是否使用扩展轴
    /// \return
    ///
    bool getExternAxis() const{
        return getCustomSetting("ExternAxis",false).toBool();
    }

    ///
    /// \brief setDhEnable  设置是否使用DH参数
    /// \param en
    ///
    void setDhEnable(bool en){
        setCustomSetting("DhEnable",en);
    }

    ///
    /// \brief setExternAxis  设置使用扩展轴
    /// \param en
    ///
    void setExternAxis(bool en){
        setCustomSetting("ExternAxis",en);
    }


    ///
    /// \brief getDhEnable  是否使用DH参数
    /// \return
    ///
    bool getDhParamEnable() const{
        return getCustomSetting("DhParamEnable",false).toBool();
    }

    ///
    /// \brief setDhEnable  设置是否使用DH参数
    /// \param en
    ///
    void setDhParamEnable(bool en){
        setCustomSetting("DhParamEnable",en);
    }

    DH getDH(quint16 id) const;

    void setDH(int id,const DH& dh);

    ///
    /// \brief getJointInfo  获取关节信息
    /// \param id
    /// \return
    ///
    JointInfo getJointInfo(quint16 id) const;

    void setJointInfo(int id,const JointInfo& info);

    ///
    /// \brief absStepFiles  绝对模型文件路径
    /// \return
    ///
    QStringList absStepFiles() const;


    void setConfig(const QString& name,
                   const QString& factory,
                   const QString& description,
                   const QString& png,
                   const QString& author,
                   const QStringList& stepFiles);

    QString getLuaScript() const;

    void setLuaScript(const QString& data);

signals:

private:
    QVariant getCustomSetting(QString key,QVariant defaultValue= QVariant()) const;

    void setCustomSetting(QString key, QVariant value);

private:
    QSettings* setting;
    QString path;
    lua_State *L = nullptr;
};

#endif // ROBOTCONFIG_H
