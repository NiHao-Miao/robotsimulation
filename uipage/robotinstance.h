// RobotInstance.h

#ifndef ROBOTINSTANCE_H
#define ROBOTINSTANCE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QGLWidget>
#include <QThread>
#include <QTimer>
#include <QList>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <AIS_Manipulator.hxx>
#include <TopoDS_TCompound.hxx>
#include <BRep_Builder.hxx>
#include "robotconfig.h"
#include "luathread.h"
#include "sceneobject.h"
#include "trajectory.h"

// 前向声明，避免循环引用
class OccView;
class LuaThread;
class RobotConfig;
class Trajectory;
class JointParam{
public:
    QVector<Handle(AIS_Shape)> shape;
    TopoDS_Shape topods_shape;
    Handle(Standard_Transient) parent_shape;
    gp_Trsf base_tr;
    gp_Ax1 theA1;
    int user_dir;
    float joint[6]; //< 平移或者旋转
    gp_Trsf tr;
    DH dh;
};
typedef struct{
    double x;
    double y;
    double z;
    double rx;
    double ry;
    double rz;
}ROBOT_POS;
// --- 关键修改 1: 在 class RobotInstance 声明之前，定义新的结构体 ---
/**
 * @brief 一个轻量级结构体，用于封装一个可被追踪的连杆的信息。
 */
struct TrackableLink {
    QString id;         // 连杆的唯一ID (例如 "Link 3", "Tool")
    gp_Trsf transform;  // 该连杆坐标系在世界坐标系中的实时位姿
};

// 定义一个记录任务 ---
struct RecordingTask {
    QString     trackedLinkId;      // 要追踪哪个连杆的ID (e.g., "Joint 5", "Tool TCP")
    Trajectory* targetTrajectory;   // 要将点记录到哪个 Trajectory 对象
};

//class RobotInstance : public QObject
// 修改类声明，使其继承自 SceneObject
class RobotInstance : public SceneObject
{
    Q_OBJECT

public:
    explicit RobotInstance(QObject *parent = nullptr);
    ~RobotInstance();

    // --- 核心方法 ---
    /**
     * @brief 从配置文件加载机器人
     * @param configPath .hcrobot文件的绝对路径
     * @param context 用于显示模型的OpenCASCADE上下文
     * @return 如果加载成功返回true
     */
    virtual bool load(const Handle(AIS_InteractiveContext)& context, const QVariant& data) override;
    /**
     * @brief 应用变换，根据当前关节状态更新所有Shape的位置
     */
    virtual void update() override;

    /**
     * @brief 更新关节角度值
     * @param joints 关节角度数组
     */
    void updateJoints(const QVector<double>& joints);

    /**
     * @brief 更新基座位姿
     * @param basePose 6D位姿 (x, y, z, rx, ry, rz)
     */
    void updateBase(const QVector<double>& basePose);

    /**
     * @brief 更新工具位姿
     * @param toolPose 6D位姿 (x, y, z, rx, ry, rz)
     */
    void updateTool(const QVector<double>& toolPose);
    // --- 新增：更新单个关节基座/偏移的函数声明 ---
    void updateJointBase(int id, const QVector<double>& pose);

    // --- 【核心修改】用新的多任务接口，替换旧的单一任务接口 ---

    /**
         * @brief 添加一个新的轨迹记录任务。
         * @param trackedLinkId 要追踪的连杆ID。
         * @param targetTrajectory 记录的目标轨迹对象。
         * @return 如果成功添加任务，返回true。
         */
    bool addRecordingTask(const QString& trackedLinkId, Trajectory* targetTrajectory);

    /**
         * @brief 根据目标轨迹对象，移除一个指定的记录任务。
         * @param targetTrajectory 要停止记录的轨迹对象。
         */
    void removeRecordingTask(Trajectory* targetTrajectory);

    /**
         * @brief 移除与指定机器人相关的所有记录任务。
         */
    void clearAllRecordingTasks();

    /**
         * @brief 检查当前是否有任何正在进行的记录任务。
         */
    bool isRecording() const;

    void addWorldPointToAllTasks(const gp_Pnt& world_point);


    virtual QString getBaseName() const override;
    virtual void setTransform(const gp_Trsf& transform) override;
    //  重写自我清理接口 ---
    virtual void removeFromScene(const Handle(AIS_InteractiveContext)& context) override;


    // --- 新增：允许外部设置实例ID的公共方法 ---
    void setId(const QString& newId);

    // --- 数据访问方法 ---
    QString getId() const;
    RobotConfig* getConfig() const;
    QVector<Handle(AIS_Shape)> getAllShapes() const;
    ROBOT_POS getCurrentTcpPos() const;
    void setVisible(bool visible, const Handle(AIS_InteractiveContext)& context);
    QVector<double> getCurrentJointValues() const;
    const QList<RecordingTask>& getRecordingTasks() const;

    // --- 关键修改 2: 在 public: 段中，添加新的公共方法声明 ---
    /**
     * @brief 获取此机器人实例所有可被追踪的连杆的列表。
     * @return 一个包含所有可追踪连杆信息的QList。
     */
    QList<TrackableLink> getTrackableLinks() const;
private:
    // 移植自OccView和RobotConfig的核心算法和数据
    QVector<Handle(AIS_Shape)> createCylindersForDHRobot();
    BOOL tr2pt(double* pt, const gp_Trsf& tr);
    void getRotationAngles(const gp_Trsf& transform, double& angleX, double& angleY, double& angleZ);
    gp_Dir dir2GpDir(int dir, int dir2 = 1);
    QVector<Handle(AIS_Shape)> makeCylinder(const gp_Pnt& pnt,const gp_Dir& dir,const gp_Dir& r_dir,double length = 500,double radius = 10);

protected:
    RobotConfig* m_config;          // 每个实例拥有独立的RobotConfig

private:
    QString m_id;                   // 唯一实例ID

    LuaThread* m_luaThread;         // 每个实例拥有独立的Lua处理器

    // --- 以下成员变量全部从 OccView 迁移而来 ---
    QMap<quint32, JointParam> m_jointShapeMap;
    QVector<JointInfo> m_jointsInfo;

    QVector<QVector<Handle(AIS_Shape)>> m_robotShapeParts; // 用于存储机器人各个部件的shape
    JointParam m_robotBase;
    JointParam m_robotTool;
    QVector<Handle(AIS_Shape)> m_tcpXYZShape;

    // --- 实时状态 ---
    ROBOT_POS m_currentTcpPos;
    QVector<double> m_currentJointValues;

    //  在 private: 段中，添加一个新的成员变量来存储连杆信息 ---
    QMap<QString, gp_Trsf> m_linkWorldTransforms; // 存储每个连杆ID到其世界变换的映射
    // 专门存储几何关键点的世界坐标 ---
    QMap<QString, gp_Pnt> m_geometricKeyPoints_World;
    // --- 用任务列表替换旧的单一指针 ---
    QList<RecordingTask> m_recordingTasks; // 存储所有正在进行的记录任务


};

#endif // ROBOTINSTANCE_H
