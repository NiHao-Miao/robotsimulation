/*
*    Copyright (c) 2018 Shing Liu All Rights Reserved.
*
*           File : occQt.h
*         Author : Shing Liu(eryar@163.com)
*           Date : 2018-01-08 20:00
*        Version : OpenCASCADE7.2.0 & Qt5.7.1
*
*    Description : OpenCASCADE in Qt.
*/

#ifndef OCCQT_H
#define OCCQT_H

#include "ui_occQt.h"
#include <QMap>     // --- 新增: 用于管理多个实例
#include <QTimer>   // --- 新增: 用于创建全局更新定时器
#include "sceneobject.h"
#include "trajectory.h"

#include <AIS_InteractiveContext.hxx>
#include "robotcommunication.h"
#include <QLabel>
#include <QStandardItemModel>
#ifdef DYNC_LIB
#include "incentivetrajectorythread.h"
#include "incentivetrajectoryrunstate.h"
#endif
#include "uipage/oscilloscopewindow.h"
#include <QFileSystemWatcher>


class RobotJoints;
class OccView;
class RobotBase;
class RobotOffset;
class LuaThread;



//! Qt main window which include OpenCASCADE for its central widget.
class occQt : public QMainWindow
{
    Q_OBJECT

public:

    static occQt* occQtIntance;

    //! constructor/destructor.
    occQt(QWidget *parent = nullptr);
    ~occQt();
    /// \brief restoreLastStatus  还原上次状态
    void restoreLastStatus();
    /// \brief initRobotModelList  加载机器人模型列表
    void initRobotModelList();
    /// \brief initRobotModelList  加载默认机器人模型列表
    void initDefaultRobotModelList();

    QStandardItemModel* getTreeModel();

    //新增定义控制模式的枚举
    enum ControlMode{
      ControlSingle,   //单体控制模式
      ControlBroadcast   //广播控制模式
    };

protected:
    //! create all the actions.
    void createActions(void);
    void createDockWidget(void);
    //! create all the menus.
    void createMenus(void);
    //! create the toolbar.
    void createToolBars(void);

private:

    void loadRobot(const QString& hcrobot);
//    void loadRobot(const QStringList &step_files);
    // 一个通用的对象加载函数 ---
    void loadObject(SceneObject* newObject, const QVariant& data);

    // --- 新增: 后续步骤将要实现的私有辅助函数 ---
    void setActiveObject(const QString& objectId);
    /// \brief robotComunicationEnable  与机器人通讯功能
    /// \param status
    void robotComunicationEnable(bool status);


    //执行移除操作
    void removeObject(const QString& ObjectId);

    void updateActionStates();




private slots:
    //! show about box.
    void about(void);
    void open();
    // --- 新增: 后续步骤将要实现的槽函数和方法 ---
    void onUpdateRequest();
    void onActiveRobotJointsChanged(QVector<double> joints);
    void onActiveRobotBaseChanged(const QVector<double>& pos);
    void onActiveRobotToolChanged(const QVector<double>& pos);
    // 用于处理模型树选择变化的槽函数 ---
    void onTreeViewSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    // 响应广播开关切换的槽函数 ---
    void onBroadcastToggled(bool checked);
    //响应移除选中机器人模型按钮的槽函数
    void onRemoveActiveObject();

private:
    Ui::occQtClass ui;

    // wrapped the widget for occ.
    OccView* myOccView;
    // --- 新增: 管理机器人实例的核心成员变量 ---
/*    QMap<QString, RobotInstance*> m_robotInstances; // 机器人实例的集合
    RobotInstance* m_activeInstance;       */         // 指向当前活动实例的指针
    QMap<QString, SceneObject*> m_sceneObjects; // 管理场景中所有对象的集合
    SceneObject* m_activeObject;                // 指向当前活动对象的指针
    QTimer* m_updateTimer;                          // 全局更新定时器
    // --- 新增: 新的成员变量控制模式 ---
    ControlMode m_controlMode;          // 存储当前控制模式
    QAction* m_broadcastAction;         // 指向“广播控制”的QAction 工具栏
    QLabel* m_controlModeLabel;         // 指向状态栏的模式标签 底部标签栏

    QAction* m_removeAction;            //创建移除选中模型的QAction

    QMap<QString,Trajectory*> m_trajectories; //管理场景中所有轨迹对象的集合
    Trajectory* m_activeTrajectory;           //指向当前用户选中的、活动的轨迹对象

private:
    RobotCommunication* robot_communication;
#ifdef DYNC_LIB
    IncentiveTrajectoryThread* incentive_trajectory_thread;
    IncentiveTrajectoryRunState* incentive_trajectory_run_state;
    QToolBar* incentive_trajectory_tool_bar;
#endif
    RobotJoints* robot_jonits;
    RobotBase* robot_base_tool;
    QLabel* network_status;
    QLabel* robot_name;
    QLabel* robot_pos;
    QLabel* view_scale;
    OscilloscopeWindow* oscilloscope_window;
    QFileSystemWatcher* file_system_watcher;
    QToolBar* trajectory_tool_bar;

    LuaThread* lua_thread;
    bool is_init;

private:
    QStandardItemModel molding_tree_view_model;

    QTimer auto_connect;
};


#endif // OCCQT_H
