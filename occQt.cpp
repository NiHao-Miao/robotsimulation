/*
*    Copyright (c) 2018 Shing Liu All Rights Reserved.
*
*           File : occQt.cpp
*         Author : Shing Liu(eryar@163.com)
*           Date : 2018-01-08 21:00
*        Version : OpenCASCADE7.2.0 & Qt5.7.1
*
*    Description : Qt main window for OpenCASCADE.
*/

#include "occQt.h"
#include "occView.h"
#include "robotinstance.h"
#include <QToolBar>
#include <QTreeView>
#include <QMessageBox>
#include <QDockWidget>
#include <QSlider>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QButtonGroup>
#include <QActionGroup >
#include "icutils.h"
#include "jointcontrol.h"
#include "robotjoints.h"
#include "systemsetting.h"
#include "robotconfig.h"
#include "createrobotconfigdialog.h"
#include "autoupdate.h"
#include "robotbase.h"
#include "jointsetting.h"
#include "moldingtreeview.h"
#include "globalconfigdialog.h"
#include "historyversionlistdialog.h"
#include "uipage/incentivetrajectorydialog.h"
#include "luathread.h"
#include "qprogressindicator.h"
#include "vendor/dync/intf/dynamics/inccurveintf.h"
#include "mqttmanage.h"
#include "staticobject.h"
#include "dynamicobject.h"
#include "trajectory.h"
#include "attachmentdialog.h"
#include "multirecorddialog.h"
#include "tool.h"

#include <QtConcurrent>
#include <QColorDialog>

#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>

#include <gp_Lin2d.hxx>

#include <Geom_ConicalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>

#include <GCE2d_MakeSegment.hxx>

#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TColgp_Array1OfPnt2d.hxx>

#include <BRepLib.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include "STEPControl_Reader.hxx"

#include <AIS_Shape.hxx>

occQt* occQt::occQtIntance = nullptr;

occQt::occQt(QWidget *parent)
    : QMainWindow(parent),
      m_controlMode(ControlSingle), // --- 新增: 默认设置为单体控制模式 ---
      m_broadcastAction(nullptr),   // --- 新增: 初始化指针 ---
//      m_activeInstance(nullptr), // --- 新增: 将活动实例指针安全地初始化为空
      m_activeObject(nullptr),
      m_updateTimer(nullptr),     // --- 新增: 将定时器指针安全地初始化为空
      robot_communication(nullptr),
      m_activeTrajectory(nullptr),
#ifdef DYNC_LIB
    incentive_trajectory_thread(nullptr),
    incentive_trajectory_run_state(nullptr),
    incentive_trajectory_tool_bar(nullptr),
#endif
    oscilloscope_window(nullptr),
    is_init(false)
{
    occQtIntance = this;
    ui.setupUi(this);
    this->setWindowTitle("华成机器人仿真平台-v"+QString(SW_VER));

    myOccView = new OccView(this);

//    connect(myOccView,&OccView::loadRobotStatus,this,[this](bool status,int a,int b){
//        ui.statusBar->showMessage("加载模型:"+RobotConfig::Instance()->name()+",进度"+QString::number(a)+"/"+QString::number(b));
//    });

    network_status = new QLabel(this);
    network_status->setStyleSheet("background-color: red;");
    robot_name = new QLabel(this);
    robot_pos = new QLabel(this);
    view_scale = new QLabel(this);

    // --- 新增: 创建并添加状态栏标签 ---
    m_controlModeLabel = new QLabel(this);
    m_controlModeLabel->setText("控制模式: 单体"); // 设置初始文本
    ui.statusBar->addPermanentWidget(m_controlModeLabel); // 添加到状态栏最右侧

    connect(myOccView,&OccView::robotPosUpdate,this,[this](const ROBOT_POS&  pos){
        robot_pos->setText(QString("X:%1 Y:%2 Z:%3 U:%4 V:%5 W:%6").arg(QString::number(pos.x, 'f', 3), 10, ' ').arg(QString::number(pos.y, 'f', 3), 10, ' ').arg(QString::number(pos.z, 'f', 3), 10, ' ').arg(QString::number(pos.rx, 'f', 3), 10, ' ').arg(QString::number(pos.ry, 'f', 3), 10, ' ').arg(QString::number(pos.rz, 'f', 3), 10, ' '));
    });
    connect(myOccView,&OccView::viewScaleChange,this,[this](double vale){
        view_scale->setText(QString("缩放:%1%").arg(vale*100));
    });
    ui.statusBar->addPermanentWidget(robot_name);
    ui.statusBar->addPermanentWidget(robot_pos);
    ui.statusBar->addPermanentWidget(view_scale);
    ui.statusBar->addPermanentWidget(network_status);

    robot_communication = new RobotCommunication(this);
    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(robot_communication,&RobotCommunication::connectStatusChanged,this,[this](bool status){
        if(status){
            network_status->setStyleSheet("background-color: green;");
        }
        else{
            network_status->setStyleSheet("background-color: red;");
        }
    });
    connect(robot_communication, &RobotCommunication::trajectorEnableChange, this, [this](bool en){
        if (en) {
            // 网络命令“开始记录”
            // 我们模拟一次“开始记录”按钮的点击逻辑，但只针对活动机器人
            RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
            if (activeRobot && !activeRobot->isRecording()) {
                Trajectory* newTrajectory = new Trajectory(this);

                loadObject(newTrajectory, QVariant());

                // b. 【默认行为】让轨迹固定在世界坐标系
                SceneObject* frameOwner = nullptr; // nullptr 代表世界

                // c. 【默认行为】让机器人追踪自己的末端工具
                QString trackedLinkId = "Tool";
                // d. 调用最新的接口启动记录
                newTrajectory->setReferenceFrame(frameOwner);
                activeRobot->addRecordingTask("Tool TCP", newTrajectory);
//                activeRobot->setActiveRecordingTrajectory(newTrajectory);
//                activeRobot->startNewRecording(frameOwner, trackedLinkId);

                m_trajectories.insert(newTrajectory->getId(), newTrajectory);
//                m_activeTrajectory = newTrajectory; // 将新轨迹设为活动
            }
        } else {
            // 网络命令“停止记录”
            // 停止所有正在记录的机器人
            for (SceneObject* obj : m_sceneObjects.values()) {
                RobotInstance* robot = dynamic_cast<RobotInstance*>(obj);
                if (robot && robot->isRecording()) {
                    robot->clearAllRecordingTasks();
                }
            }
        }
        updateActionStates(); // 无论开始还是停止，都更新UI状态
    });
    connect(robot_communication, &RobotCommunication::trajectorColorChange, this, [this](const QColor& color){
        // 网络命令总是作用于“当前活动”的轨迹上
        if (m_activeTrajectory) {
            m_activeTrajectory->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
        }
    });
    connect(robot_communication, &RobotCommunication::updateRobotWorld, this, [this](QVector<double> world){
        // 这个信号的意图是画一个世界坐标点。在我们的新架构下，它应该画在“世界轨迹”上
        // 为了实现这一点，我们需要找到或创建一个参考系为“世界”的轨迹

        // 简化逻辑：我们让这个信号作用于所有正在录制的机器人
        // 注意：这可能不是最终想要的行为，但它兼容了原有功能
        gp_Pnt world_pnt(world[0], world[1], world[2]);
        for (SceneObject* obj : m_sceneObjects.values()) {
            RobotInstance* robot = dynamic_cast<RobotInstance*>(obj);
            // 如果机器人正在录制，就调用它的 recordNewPoint
            if (robot && robot->isRecording()) {
                robot->addWorldPointToAllTasks(world_pnt);
            }
        }
    });

    file_system_watcher = new QFileSystemWatcher(this);
    QDir dir("robot");
    file_system_watcher->addPath(dir.absolutePath());
    connect(file_system_watcher,&QFileSystemWatcher::fileChanged,this,[this](){
        initRobotModelList();
    });
    connect(file_system_watcher,&QFileSystemWatcher::directoryChanged,this,[this](){
        initRobotModelList();
    });

    setCentralWidget(myOccView);
    createActions();
    createMenus();
    createToolBars();

    //< 默认不开启网络

    robotComunicationEnable(SystemSetting::Instance()->getLastNetworkState());
    //新增根据状态，同步UI按钮的勾选情况
    if(SystemSetting::Instance()->getLastNetworkState()){
        ui.actionremote_enable->setChecked(true);
    }else {
        ui.actionremote_disable->setChecked(true);
    }

    const int width = 300;
    QDockWidget* left_dock = new QDockWidget(this);
    left_dock->setWindowTitle("模型树");
    left_dock->setMinimumWidth(width);
    addDockWidget(Qt::LeftDockWidgetArea,left_dock);

    MoldingTreeView* molding_tree_view = new MoldingTreeView(left_dock);
    molding_tree_view->setModel(&molding_tree_view_model);
    // 连接选择变化信号到我们新创建的槽函数 ---
    connect(molding_tree_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &occQt::onTreeViewSelectionChanged);
    lua_thread = new LuaThread();

    connect(MqttManage::Instance(),&MqttManage::onOnline,this,[this](const QString& name){
        ui.statusBar->showMessage("通知:有人上线了！"+name,5000);
    });

    connect(&auto_connect,&QTimer::timeout,this,[this](){
        bool auto_connect_enable = SystemSetting::Instance()->getNetworkAutoConnect();
        if(!robot_communication->isconnect() && auto_connect_enable){
            robot_communication->connect(SystemSetting::Instance()->getTcpNetIpAddr(),SystemSetting::Instance()->getTcpNetPort());
            robot_communication->startThread();
        }
    });
    // --- 在构造函数的末尾，m_updateTimer 初始化之前，添加这一行 ---
    createDockWidget();
    // --- 新增: 初始化并启动全局更新定时器 ---
    // 这个定时器将取代原OccView中的定时器，成为唯一的渲染驱动
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &occQt::onUpdateRequest);
    m_updateTimer->start(16); // 设置约60FPS的更新频率 (1000ms / 60 ≈ 16ms)
}

occQt::~occQt()
{
    qDeleteAll(m_sceneObjects);
    m_sceneObjects.clear();
    // 析构函数中 qDeleteAll(m_sceneObjects) 会自动删除所有轨迹对象，
    // 因为 Trajectory 也是 SceneObject。
    // 所以我们只需要 clear 即可。
    m_trajectories.clear();

    delete  lua_thread;
    if(oscilloscope_window){
        delete oscilloscope_window;
    }
}


void occQt::createActions( void )
{
    // Primitive
    connect(ui.actionBox, SIGNAL(triggered()), myOccView, SLOT(makeBox()));
    connect(ui.actionCone, SIGNAL(triggered()), myOccView, SLOT(makeCone()));
    connect(ui.actionSphere, SIGNAL(triggered()), myOccView, SLOT(makeSphere()));
    connect(ui.actionCylinder, SIGNAL(triggered()), myOccView, SLOT(makeCylinder()));
    connect(ui.actionTorus, SIGNAL(triggered()), myOccView, SLOT(makeTorus()));

    // File
    connect(ui.actionopen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

    //< 视图
    connect(ui.actionReset, SIGNAL(triggered()), myOccView, SLOT(reset()));
    connect(ui.actionFitAll, SIGNAL(triggered()), myOccView, SLOT(fitAll()));

    //< 切换视图
    connect(ui.actionfrontview, SIGNAL(triggered()), myOccView, SLOT(onButtonFrontview()));
    connect(ui.actionrearview, SIGNAL(triggered()), myOccView, SLOT(onButtonRearview()));
    connect(ui.actionleftview, SIGNAL(triggered()), myOccView, SLOT(onMenuLeftview()));
    connect(ui.actionrightview, SIGNAL(triggered()), myOccView, SLOT(onButtonRightview()));
    connect(ui.actiontopview, SIGNAL(triggered()), myOccView, SLOT(onMenuTopview()));
    connect(ui.actionupview, SIGNAL(triggered()), myOccView, SLOT(onButtonUpview()));
    connect(ui.actionAxoview, SIGNAL(triggered()), myOccView, SLOT(onMenuAxo()));

    //< 显示模式
    connect(ui.actionwireframe, SIGNAL(triggered()), myOccView, SLOT(onButtonWireframemode()));
    connect(ui.actionshaded, SIGNAL(triggered()), myOccView, SLOT(onButtonEntitemode()));
    ui.actionshaded->setChecked(true);
    QActionGroup  *group = new QActionGroup (this);
    group->addAction(ui.actionwireframe);
    group->addAction(ui.actionshaded);

    //< 背景切换
    connect(ui.actiongray, SIGNAL(triggered()), myOccView, SLOT(setGrayBg()));
    connect(ui.actionBg_Black, SIGNAL(triggered()), myOccView, SLOT(setBlackBg()));
    ui.actiongray->setChecked(true);
    QActionGroup  *bg_group = new QActionGroup (this);
    bg_group->addAction(ui.actiongray);
    bg_group->addAction(ui.actionBg_Black);

    //< 网格
    connect(ui.actionGrid_enable, SIGNAL(triggered()), myOccView, SLOT(GridEnable()));
    connect(ui.actionGrid_disable, SIGNAL(triggered()), myOccView, SLOT(GridDisable()));
    ui.actionGrid_enable->setChecked(true);
    QActionGroup  *grid_group = new QActionGroup (this);
    grid_group->addAction(ui.actionGrid_enable);
    grid_group->addAction(ui.actionGrid_disable);

    //< 模型
    connect(ui.actionimport, &QAction::triggered, this, [this](){
        CreateRobotConfigDialog* dialog = new CreateRobotConfigDialog();
        int ret = dialog->exec();
        if(ret == QDialog::Accepted){
            //< 确定
            if(dialog->name().size()==0){
                delete dialog;
                QMessageBox::warning(nullptr,"创建模型失败","模型名字不能为空!");
                return ;
            }
            if(dialog->jointNumber()==0){
                delete dialog;
                QMessageBox::warning(nullptr,"创建模型失败","关节数量不能为0!");
                return ;
            }
            //< 拷贝模型文件到指定目录下
            RobotConfig* robot = new RobotConfig();
            ICUtils::copyRobotModelFiles(dialog->name(),dialog->absStepFiles());
            robot->loadRobotConfig(dialog);
            delete robot;
            QMessageBox::information(nullptr,"创建模型成功","创建模型"+dialog->name()+"成功!");

            //< 刷新模型列表
            initRobotModelList();
        }
        delete dialog;
    });

    //< 模型-> 配置 -> 机器人配置
    connect(ui.actionRobotConfig, &QAction::triggered, this, [this](){
        RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject); // <-- 修改
        if(!activeRobot){ // 修改为此行：检查是否存在活动的机器人实例
            QMessageBox::warning(nullptr,"机器人参数配置","当前未加载任何机器人模型，无法修改配置.");
            return ;
        }
        // 后续代码应使用 m_activeInstance->getConfig() 来获取配置
        CreateRobotConfigDialog* dialog = new CreateRobotConfigDialog(nullptr, "机器人参数配置", activeRobot->getConfig());
        int ret = dialog->exec();
        if(ret == QDialog::Accepted){
            //< 确定
            if(dialog->name().size()==0){
                delete dialog;
                QMessageBox::warning(nullptr,"创建模型失败","模型名字不能为空!");
                return ;
            }
            if(dialog->jointNumber()==0){
                delete dialog;
                QMessageBox::warning(nullptr,"创建模型失败","关节数量不能为0!");
                return ;
            }
            RobotConfig* robot = new RobotConfig();
            ICUtils::copyRobotModelFiles(dialog->name(),dialog->absStepFiles());
            robot->loadRobotConfig(dialog);
            initRobotModelList();
            QString robotAbsPath =robot->robotAbsPath();
            delete robot;
            QMessageBox::information(nullptr,"机器人参数配置","修改机器人"+dialog->name()+"成功!需要重新加载.");
            /// 重新加载模型
            SystemSetting::Instance()->setCustomSetting("last_hcrobot",robotAbsPath);
            this->loadRobot(robotAbsPath);
        }
        delete dialog;
    });
    //新增，创建移除选中模型的QAction
    m_removeAction = new QAction("移除选中模型",this);
    m_removeAction->setToolTip("从场景和模型树中移除当前选中的机器人模型");
    m_removeAction->setEnabled(false);
    connect(m_removeAction,&QAction::triggered,this,&occQt::onRemoveActiveObject);
    ui.menuModeling->addSeparator();
    ui.menuModeling->addAction(m_removeAction);

    //< 物体
    //添加静态桌面
    connect(ui.actionAdd_Static_Table,&QAction::triggered,this,[this](){
        //创建一个新的statciobject实例
        StaticObject* table = new StaticObject(this);
        //调用通用的加载函数loadobject
        //对于简单的物体，我们不需要传递额外数据，所以QVariant是空的
        loadObject(table,QVariant());
    });
    //添加动态立方体
    connect(ui.actionAdd_Dynamic_Cube,&QAction::triggered,this,[this](){
        //创建一个新的动态实例
        DynamicObject* cube = new DynamicObject(this);
        //调用通用的加载函数
        loadObject(cube,QVariant());
    });

    //< 工具
    //加载虚拟工具
    connect(ui.actionLoadTool,&QAction::triggered,this,[this](){
        // 1. 创建一个新的 Tool 对象实例
        Tool* virtualGripper = new Tool(this);
        // 2. 指定我们之前创建的虚拟工具的配置文件路径
        //    这里假设 VirtualGripper.hcrobot 文件在 "robot/" 目录下
        QString configPath = "robot/VirtualGripper.hcrobot";
        // 3. 调用通用的加载函数 loadObject
        loadObject(virtualGripper, QVariant(configPath));


    });

    //< 轨迹
    connect(ui.actionStartrecord, &QAction::triggered, this, [this](){
        // --- 功能点 1: 前置检查 ---
        RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
        if (!activeRobot) {
            QMessageBox::warning(this, "操作失败", "请先在模型树中选择一个机器人，然后再开始记录轨迹。");
            ui.actionStartrecord->setChecked(false);
            return;
        }
//        // --- 功能点 2: 避免重复 ---
//        if (activeRobot->isNewRecording()) {
//            QMessageBox::information(this, "提示", "当前选中的机器人已经在记录轨迹中。");
//            return;
//        }

        // --- 准备对话框的数据 ---
        // 1. 获取机器人的可追踪连杆列表
        QList<TrackableLink> links = activeRobot->getTrackableLinks();
        QStringList linkIds;
        for (const auto& link : links) {
            linkIds.append(link.id);
        }
        // 2. 准备可附着对象列表
//        QStringList attachmentTargetNames;
//        attachmentTargetNames.append("世界坐标系 (World)");
//        for (SceneObject* obj : m_sceneObjects.values()) {
//            // 使用 dynamic_cast 来检查对象的真实类型
//            // 如果这个对象不是一个 Trajectory 对象，那么它就是一个有效的附着目标
//            if (dynamic_cast<Trajectory*>(obj) == nullptr) {
//                attachmentTargetNames.append(obj->getId());
//            }
//        }
        QStringList frameNames;
        frameNames.append("世界坐标系(World)");
        for(SceneObject* obj : m_sceneObjects.values()){
            // 使用 dynamic_cast 来检查对象的真实类型
            // 如果这个对象不是一个 Trajectory 对象，那么它就是一个有效的附着目标
            if(dynamic_cast<Trajectory*>(obj) == nullptr){
                frameNames.append(obj->getId());
            }
        }

        //  创建并显示对话框
        MultiRecordDialog dialog(this);
        dialog.setWindowTitle("配置多点轨迹记录");
        dialog.setLinks(linkIds);
        dialog.setReferenceFrames(frameNames);

        if (dialog.exec() == QDialog::Accepted) {
            QStringList selectedLinks = dialog.getSelectedLinks(); // <-- 获取用户选择的连杆
            QString selectedTargetId = dialog.getSelectedReferenceFrame();
            if (selectedLinks.isEmpty()) {
                return; // 用户没有选择任何连杆
            }
            SceneObject* frameOwner = nullptr;
            if (selectedTargetId != "世界坐标系 (World)") {
                frameOwner = m_sceneObjects.value(selectedTargetId, nullptr);
            }

            // --- 功能点 3: 为每一个被选中的连杆，创建轨迹并启动记录任务 ---
            for(const QString& linkId : selectedLinks){
                Trajectory* newTraj = new Trajectory(this);

                // --- 功能点 4: 智能命名 ---
                QString baseName = activeRobot->getId() + "_" /*+ linkId.replace(" ", "_")*/;
                if (frameOwner) {
                    baseName = baseName + "_on_" + frameOwner->getId()+ "_traj";
                }
                QString trajId = baseName;
                int counter = 1;
                while(m_sceneObjects.contains(trajId)) {
                    trajId = QString("%1_%2").arg(baseName).arg(counter);
                    counter++;
                }
                newTraj->setId(trajId);

                // --- 功能点 5: 加载到场景 ---
                loadObject(newTraj, QVariant());

                // --- 功能点 6: 设置显示参考系 ---
                newTraj->setReferenceFrame(frameOwner);

                // --- 功能点 8: 建立绑定 ---
                //            activeRobot->setActiveRecordingTrajectory(newTrajectory);

                // --- 功能点 7: 启动机器人记录 ---
                activeRobot->addRecordingTask(linkId, newTraj);

                // --- 功能点 9: 管理轨迹对象 ---
                m_trajectories.insert(newTraj->getId(), newTraj);
            }
            updateActionStates();
            // --- 功能点 10: UI反馈 ---
            // (切换选中项的代码，隐式地调用了 onTreeViewSelectionChanged，
            //  它会更新 m_activeTrajectory 并调用 updateActionStates)
//            for (int i = 0; i < molding_tree_view_model.rowCount(); ++i) {
//                if (molding_tree_view_model.item(i)->text() == newTrajectory->getId()) {
//                    QModelIndex index = molding_tree_view_model.index(i, 0);
//                    MoldingTreeView* treeView = findChild<MoldingTreeView*>();
//                    if (treeView) {
//                        treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
//                    }
//                    break;
//                }
//            }
//            ui.statusBar->showMessage("已为 " + activeRobot->getId() + " 开始记录新轨迹: " + newTrajectory->getId(), 5000);

//        } else {
//            // (处理用户点击“取消”的情况)
//            ui.actionStartrecord->setChecked(false);
        }
    });

    connect(ui.actionStoprecord, &QAction::triggered, this, [this](){
        RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
        if (activeRobot) {
            activeRobot->clearAllRecordingTasks(); // 调用新接口，清空所有任务
            ui.statusBar->showMessage("已停止 " + activeRobot->getId() + " 的所有轨迹记录", 3000);
        }
        updateActionStates();
    });
    ui.actionStoprecord->setChecked(true);
    QActionGroup  *trajectory_group = new QActionGroup (this);
    trajectory_group->addAction(ui.actionStartrecord);
    trajectory_group->addAction(ui.actionStoprecord);

    connect(ui.actionCleartrajectory, &QAction::triggered, this, [this](){
        // 清除当前被选中的“活动轨迹”
        if (m_activeTrajectory) {
            m_activeTrajectory->clear();
            ui.statusBar->showMessage("已清除活动轨迹 " + m_activeTrajectory->getId(), 3000);
        } else {
            // (可选) 如果没有活动轨迹，可以询问用户是否要清除所有轨迹
            QMessageBox::information(this, "提示", "请先在左侧模型树中选择一条轨迹，然后再执行清除操作。");
        }
    });

    //< 导出轨迹
    connect(ui.actionExporttrajectory, &QAction::triggered, this, [this](){
        if (!m_activeTrajectory) {
            QMessageBox::warning(this, "操作失败", "请先在模型树中选择一条要导出的轨迹。");
            return;
        }
        QStringList points =  m_activeTrajectory->exportPoints();
        // 如果轨迹为空，给用户一个提示
        if (points.isEmpty()) {
            QMessageBox::information(this, "提示", "当前选择的轨迹中没有可导出的点。");
            return;
        }
        //QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
        //                   "/home/jana/untitled.png",
        //                    r("Images (*.png *.xpm *.jpg)"));

        QString file_path = QFileDialog::getSaveFileName(nullptr, QObject::tr("保存轨迹"),
                                                         QDir::homePath(), QObject::tr("华成机器人轨迹文件 (*.hcrobot_points)"));
        // 将数据写入文件
        QFile file(file_path);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)){ // 使用 WriteOnly 和 Text 模式
            QTextStream textStream(&file);
            for (const QString &str : points) {
                textStream << str << "\n";
            }
            file.close();
            QMessageBox::information(nullptr,"保存成功","轨迹文件 '" + QFileInfo(file).fileName() + "' 保存成功!");
        }
        else{
            QMessageBox::warning(nullptr,"保存失败","保存轨迹失败，请检查目录权限或文件名是否有效！");
        }
    });

    //< 导入轨迹
    connect(ui.actionimporttrajectory, &QAction::triggered, this, [this](){
        QString file_path = QFileDialog::getOpenFileName(nullptr, QObject::tr("打开轨迹"),
                                                         QDir::homePath(), QObject::tr("华成机器人轨迹文件 (*.hcrobot_points)"));
        // 检查用户是否取消了选择
        if (file_path.isEmpty()) {
            return; // 用户点击了取消，直接返回
        }
        // 3. (逻辑不变) 读取文件内容到 QStringList
        QFile file(file_path);
        QStringList points;
        if(file.open(QFile::ReadOnly | QFile::Text)){ // 使用 ReadOnly 和 Text 模式
            QTextStream in(&file);
            while(!in.atEnd()){
                points.append(in.readLine());
            }
            file.close();

            if (points.isEmpty()) {
                // 兼容可能只有一行且没有换行符的文件
                file.open(QFile::ReadOnly | QFile::Text);
                points = QString(file.readAll()).split("\n");
                file.close();
                points.removeAll(""); // 移除可能产生的空字符串
            }

        } else {
            QMessageBox::warning(nullptr,"打开失败","轨迹文件打开失败! 请检查文件是否存在或是否有读取权限!");
            return;
        }

        // 4. (逻辑不变) 检查文件是否为空或格式不正确
        if (points.isEmpty() || (points.size() == 1 && points.first().isEmpty())) {
            QMessageBox::warning(nullptr,"导入失败","轨迹文件中内容为空或格式不正确!");
            return;
        }

        // 5. 【核心修改】创建新的 Trajectory 对象来承载导入的数据
        //    a. 创建一个新的 Trajectory 对象实例
        Trajectory* importedTraj = new Trajectory(this);

        //    b. 调用 Trajectory 自己的 importPoints 方法，让它自己解析并填充点
        importedTraj->importPoints(points);

        //    c. 将这个新的 Trajectory 对象加载到我们的通用场景管理系统中
        loadObject(importedTraj, QVariant());

        //    d. 默认让导入的轨迹固定在世界坐标系（不跟随任何物体）
        importedTraj->setReferenceFrame(nullptr);

        //    e. 将新创建的 Trajectory 对象添加到我们的专属管理器中
        m_trajectories.insert(importedTraj->getId(), importedTraj);

        QMessageBox::information(nullptr,"导入成功","轨迹文件已成功导入到场景中。");
    });


    //< 示波器
    connect(ui.oscilloscope, &QAction::triggered, this, [this](){
        oscilloscope_window = new OscilloscopeWindow();
        oscilloscope_window->show();
        connect(myOccView,&OccView::addPoint,oscilloscope_window,&OscilloscopeWindow::appendPoint);
    });

    //< 网络
    connect(ui.actionremote_enable, &QAction::triggered, this, [this](){
        robotComunicationEnable(true);
    });
    connect(ui.actionremote_disable, &QAction::triggered, this, [this](){
        robotComunicationEnable(false);
    });
    ui.actionremote_enable->setCheckable(true);
    ui.actionremote_disable->setCheckable(true);
    QActionGroup  *network_group = new QActionGroup (this);
    network_group->addAction(ui.actionremote_enable);
    network_group->addAction(ui.actionremote_disable);

    m_broadcastAction = new QAction("广播模式",this);
    m_broadcastAction->setToolTip("开启后，将同时控制所有加载的模型");
    m_broadcastAction->setCheckable(true);// 设置为可勾选
    m_broadcastAction->setEnabled(false);// 默认禁用，因为启动时没有足够的模型
    ui.menuNetwork->addSeparator(); // 在菜单中加一个分隔符
    ui.menuNetwork->addAction(m_broadcastAction); // 添加新菜单项
    connect(m_broadcastAction,&QAction::toggled,this,&occQt::onBroadcastToggled);

    //< 设置编辑模式
    connect(ui.actionEdit_enable, &QAction::triggered, this, [this](){
        myOccView->setEditMode(true);
        ui.actionEdit_disable->setChecked(false);
    });
    connect(ui.actionEdit_disable, &QAction::triggered, this, [this](){
        myOccView->setEditMode(false);
        ui.actionEdit_enable->setChecked(false);
    });


    // Help
    connect(ui.actionconfig,&QAction::triggered, this, [this](){
        GlobalConfigDialog* global_config = new GlobalConfigDialog(this);
        global_config->open();
    });
    //< 历史版本
    connect(ui.actionHistoryVersion,&QAction::triggered, this, [this](){
        HistoryVersionListDialog* dialog = new HistoryVersionListDialog(this);
        dialog->open();
    });
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.actionCheck_updae,&QAction::triggered, this, [](){
        bool ret  = ICUtils::isWebOk();
        if(!ret){
            QMessageBox::warning(nullptr,"网络错误","无法连接到服务器，请检查网络后重试!");
            return ;
        }
        AutoUpdate::Instance()->checkUpdate(true);
    });
    connect(ui.actionAboutQt,&QAction::triggered, this, [](){
        QMessageBox::aboutQt(nullptr,"关于QT");
    });


}

void occQt::createDockWidget(void)
{
    if(is_init) return;
    is_init = true;

    // --- 原有的创建Dock和UI面板的代码保持不变 ---
    const int width = 350;
    QDockWidget* right_dock = new QDockWidget(this);
    right_dock->setWindowTitle("关节控制");
    right_dock->setMinimumWidth(width);
    addDockWidget(Qt::RightDockWidgetArea,right_dock);
    robot_jonits = new RobotJoints(right_dock);

    QDockWidget* right_dock1 = new QDockWidget(this);
    right_dock1->setWindowTitle("基座和工具");
    right_dock1->setMinimumWidth(width);
    addDockWidget(Qt::RightDockWidgetArea,right_dock1);
    robot_base_tool = new RobotBase(right_dock1);

    // --- 关键修改：更新信号槽连接以匹配新架构 ---

    // a. 将UI面板的信号连接到occQt的新槽函数上
    connect(robot_jonits, &RobotJoints::valueChange, this, &occQt::onActiveRobotJointsChanged);
    connect(robot_base_tool, &RobotBase::updateBase, this, &occQt::onActiveRobotBaseChanged);
    connect(robot_base_tool, &RobotBase::updateTool, this, &occQt::onActiveRobotToolChanged);

    // b. 更新机器人网络通信的信号槽连接逻辑
    connect(robot_communication, &RobotCommunication::updateRobotJoints, this, [this](QVector<double> joints){
        // 当收到网络数据时，只对当前活动机器人进行操作
        if (m_controlMode == ControlBroadcast) {
            // 首先用网络数据更新UI滑块的位置
            RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
            if(activeRobot) {
                robot_jonits->setJointsValue(joints);
            }
            //            robot_jonits->setJointsValue(joints);
        }else {
            RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
            if (activeRobot) {
                robot_jonits->setJointsValue(joints);
                // 如果没有活动模型但有其他模型，可以让UI反映第一个模型的状态
                //            robot_jonits->setJointsValue(joints);

                // 然后，也需要更新活动实例的内部数据
                // (注：原代码只更新了UI，这里我们闭合了数据环路)
                // --- 关键修改：不再直接调用updateJoints，而是让UI去触发 ---
                // `setJointsValue`会触发`robot_jonits`的`valueChange`信号，
                // 该信号连接到`onActiveRobotJointsChanged`槽，
                // `onActiveRobotJointsChanged`内部已经包含了广播/单体的判断逻辑。
                // 这样就统一了手动操作和网络操作的逻辑入口。
                // m_activeInstance->updateJoints(joints);


                // 处理扩展轴的逻辑
                if (activeRobot->getConfig() && activeRobot->getConfig()->getExternAxis()) {
                    // TODO: 需要在RobotInstance中实现类似luaUpdateBase的功能
                    // robot_base_tool->setBaseJoints(m_activeInstance->luaUpdateBase(joints));
                }
            }
        }
    });

    tabifyDockWidget(right_dock, right_dock1);
}

void occQt::createMenus( void )
{
}

void occQt::createToolBars( void )
{
    QToolBar* aToolBar = addToolBar(tr("&Navigate"));
    aToolBar->addAction(ui.actionReset);
    aToolBar->addAction(ui.actionFitAll);

    aToolBar = addToolBar(tr("&Primitive"));
    aToolBar->addAction(ui.actionBox);
    aToolBar->addAction(ui.actionCone);
    aToolBar->addAction(ui.actionSphere);
    aToolBar->addAction(ui.actionCylinder);
    aToolBar->addAction(ui.actionTorus);

    trajectory_tool_bar = addToolBar("轨迹");
    trajectory_tool_bar->addAction(ui.actionStartrecord);
    trajectory_tool_bar->addAction(ui.actionStoprecord);
    trajectory_tool_bar->addAction(ui.actionCleartrajectory);
    QActionGroup* action_group = new QActionGroup(this);
    //< 颜色选择
    QAction* color_action= new QAction(this);
    color_action->setText("轨迹颜色");
    color_action->setCheckable(true);
    connect(color_action,&QAction::triggered,this,[this](){
        // 1. 检查当前是否有活动的轨迹被选中
        if (!m_activeTrajectory) {
            QMessageBox::information(this, "提示", "请先在左侧模型树中选择一条轨迹，然后再设置颜色。");
            return;
        }
        QColorDialog* color_dialog = new QColorDialog(this);
        color_dialog->setWindowTitle("轨迹颜色选择");
        color_dialog->setCurrentColor(QColor("yellow"));
        int ret =  color_dialog->exec();
        if(ret == QDialog::Accepted){
            QColor color = color_dialog->currentColor();
            m_activeTrajectory->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
//            myOccView->setPointColor(color.redF(),color.greenF(),color.blueF());
//            m_activeInstance->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
        }
        delete color_dialog;
    });
    trajectory_tool_bar->addAction(color_action);
    action_group->addAction(color_action);

    QAction* green_color_action= new QAction(this);
    green_color_action->setText("绿");
    green_color_action->setCheckable(true);
    connect(green_color_action,&QAction::triggered,this,[this](){
        if (m_activeTrajectory) {
            m_activeTrajectory->setTrajectoryColor(Quantity_NOC_GREEN);
        }
//        QColor color = QColor("green");
//        myOccView->setPointColor(color.redF(),color.greenF(),color.blueF());
//        m_activeInstance->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
    });
    trajectory_tool_bar->addAction(green_color_action);
    action_group->addAction(green_color_action);

    QAction* red_color_action= new QAction(this);
    red_color_action->setText("红");
    red_color_action->setCheckable(true);
    connect(red_color_action,&QAction::triggered,this,[this](){
        if (m_activeTrajectory) {
            m_activeTrajectory->setTrajectoryColor(Quantity_NOC_RED);
        }
//        QColor color = QColor("red");
//        myOccView->setPointColor(color.redF(),color.greenF(),color.blueF());
//        m_activeInstance->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
    });
    trajectory_tool_bar->addAction(red_color_action);
    action_group->addAction(red_color_action);

    QAction* yellow_color_action= new QAction(this);
    yellow_color_action->setText("黄");
    yellow_color_action->setCheckable(true);
    connect(yellow_color_action,&QAction::triggered,this,[this](){
        if (m_activeTrajectory) {
            m_activeTrajectory->setTrajectoryColor(Quantity_NOC_YELLOW);
        }
//        QColor color = QColor("yellow");
//        myOccView->setPointColor(color.redF(),color.greenF(),color.blueF());
//        m_activeInstance->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
    });
    trajectory_tool_bar->addAction(yellow_color_action);
    action_group->addAction(yellow_color_action);

    QAction* blue_color_action= new QAction(this);
    blue_color_action->setText("蓝");
    blue_color_action->setCheckable(true);
    connect(blue_color_action,&QAction::triggered,this,[this](){
        if (m_activeTrajectory) {
            m_activeTrajectory->setTrajectoryColor(Quantity_NOC_BLUE);
        }
//        QColor color = QColor("blue");
//        myOccView->setPointColor(color.redF(),color.greenF(),color.blueF());
//        m_activeInstance->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
    });
    trajectory_tool_bar->addAction(blue_color_action);
    action_group->addAction(blue_color_action);

    connect(action_group,&QActionGroup::triggered,this,[this](QAction *action){

    });

    QAction* clear_color_action= new QAction(this);
    clear_color_action->setText("清除选中颜色轨迹");
    connect(clear_color_action,&QAction::triggered,this,[this](){
        QMessageBox::information(this, "功能待定", "按颜色清除轨迹的功能将在后续步骤中重新设计和实现。");
//        myOccView->ClearColorRecordTrajectory();l
    });
    trajectory_tool_bar->addAction(clear_color_action);

    QToolBar* network_tool_bar = addToolBar("网络");
    network_tool_bar->addAction(ui.actionremote_enable);
    network_tool_bar->addAction(ui.actionremote_disable);
    network_tool_bar->addSeparator();
    network_tool_bar->addAction(m_broadcastAction);

    //< 默认值
    green_color_action->setChecked(true);
//    QColor color = QColor("red");
//    myOccView->setPointColor(color.redF(),color.greenF(),color.blueF());
//    m_activeInstance->setTrajectoryColor(Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB));
}

void occQt::restoreLastStatus()
{
    if(SystemSetting::Instance()->getOpenLastProject()){
        //< 加载上次打开的文件
        QString last_path =  SystemSetting::Instance()->getCustomSetting("last_hcrobot",QString()).toString();
        this->loadRobot(last_path);
    }
}

void occQt::initRobotModelList()
{
    qDebug()<<"initRobotModelList Start";
    //<
    ui.menuload->clear();
    //< 加载模型列表
    QDir dir;
    if(!dir.cd("robot")){
        dir.mkdir("robot");
        dir.cd("robot");
    }
    QStringList files = ICUtils::getPathFileList(dir.absolutePath(),"*.hcrobot",true);
    class RobotModel{
    public:
        QString name;
        QString dir_path;
        QString file_path;
    };
    QMap<QString,QVector<RobotModel> > robot_list;
    for(int i = 0,len = files.size();i<len;i++){
        QFileInfo file(files.at(i));
        RobotConfig* robot = new RobotConfig();
        robot->loadRobotConfig(file.absoluteFilePath());
        RobotModel model;
        model.name = robot->name();
        model.file_path = file.absoluteFilePath();
        QDir model_dir(dir);
        model_dir.cd(robot->name());
        model.dir_path = model_dir.absolutePath();
        robot_list[robot->factory()].append(model);
        delete robot;
    }
    for(int i = 0;i<robot_list.size();i++){
        QMenu *menu = new QMenu(this);
        menu->setTitle(robot_list.keys().at(i));
        ui.menuload->addMenu(menu);
        QVector<RobotModel>  values = robot_list.values()[i];
        for(int j = 0;j<values.size();j++){

            QMenu *robot_menu = new QMenu(this);
            robot_menu->setTitle(values[j].name);
            menu->addMenu(robot_menu);

            QAction* action = new QAction(this);
            action->setText("加载");
            robot_menu->addAction(action);
            connect(action, &QAction::triggered, this, [this,values,j](){
                this->loadRobot(values[j].file_path);
            });
            QAction* action1 = new QAction(this);
            action1->setText("修改");
            robot_menu->addAction(action1);
            connect(action1, &QAction::triggered, this, [this,values,j](){
                //< 修改模型
                RobotConfig* robot_config = new RobotConfig();
                robot_config->loadRobotConfig(values[j].file_path);
                CreateRobotConfigDialog* dialog = new CreateRobotConfigDialog(nullptr,"机器人参数配置",robot_config);
                int ret = dialog->exec();
                if(ret == QDialog::Accepted){
                    //< 确定
                    if(dialog->name().size()==0){
                        delete dialog;
                        delete robot_config;
                        QMessageBox::warning(nullptr,"修改模型失败","模型名字不能为空!");
                        return ;
                    }
                    if(dialog->jointNumber()==0){
                        delete dialog;
                        delete robot_config;
                        QMessageBox::warning(nullptr,"修改模型失败","关节数量不能为0!");
                        return ;
                    }
                    ICUtils::copyRobotModelFiles(dialog->name(),dialog->absStepFiles());
                    robot_config->loadRobotConfig(dialog);
                }
                delete dialog;
                delete robot_config;
                initRobotModelList();
            });
            QAction* action2 = new QAction(this);
            action2->setText("删除");
            robot_menu->addAction(action2);
            connect(action2, &QAction::triggered, this, [this,values,j](){
                //< 删除模型
                QFile file(values[j].file_path);
                file.remove();
                ICUtils::deleteDirectory(values[j].dir_path);
                /// 重新加载模型列表
                initRobotModelList();
            });

        }
    }
    qDebug()<<"initRobotModelList End";
    ui.statusBar->showMessage("模型列表已重新加载!",10);

}

void occQt::initDefaultRobotModelList()
{
    qDebug()<<"initRobotModelList Start";
    //<
    ui.menuload->clear();
    //< 加载模型列表
    QDir dir;
    if(!dir.cd("default_robot")){
        dir.mkdir("default_robot");
        dir.cd("default_robot");
    }
    QStringList files = ICUtils::getPathFileList(dir.absolutePath(),"*.hcrobot",true);
    class RobotModel{
    public:
        QString name;
        QString dir_path;
        QString file_path;
    };
    QMap<QString,QVector<RobotModel> > robot_list;
    for(int i = 0,len = files.size();i<len;i++){
        QFileInfo file(files.at(i));
        RobotConfig* robot = new RobotConfig();
        robot->loadRobotConfig(file.absoluteFilePath());
        RobotModel model;
        model.name = robot->name();
        model.file_path = file.absoluteFilePath();
        QDir model_dir(dir);
        model_dir.cd(robot->name());
        model.dir_path = model_dir.absolutePath();
        robot_list[robot->factory()].append(model);
        delete robot;
    }
    for(int i = 0;i<robot_list.size();i++){
        QMenu *menu = new QMenu(this);
        menu->setTitle(robot_list.keys().at( i));
        ui.default_robot_2->addMenu(menu);
        QVector<RobotModel>  values = robot_list.values()[i];
        for(int j = 0;j<values.size();j++){

            QMenu *robot_menu = new QMenu(this);
            robot_menu->setTitle(values[j].name);
            menu->addMenu(robot_menu);

            QAction* action = new QAction(this);
            action->setText("加载");
            robot_menu->addAction(action);
            connect(action, &QAction::triggered, this, [this,values,j](){
                this->loadRobot(values[j].file_path);
            });
            QAction* action1 = new QAction(this);
            action1->setText("复制到用户模型列表");
            robot_menu->addAction(action1);
            connect(action1, &QAction::triggered, this, [this,values,j](){
                //< TODO:复制到用户模型列表
                ICUtils::copyRobot(values[j].file_path);
                QMessageBox::information(nullptr, "模型拷贝成功","模型拷贝成功,可以在模型列表中加载模型！");
            });

        }
    }
    qDebug()<<"initRobotModelList End";
    ui.statusBar->showMessage("默认模型列表已重新加载!",10);
}

QStandardItemModel *occQt::getTreeModel()
{
    return &molding_tree_view_model;
}

void occQt::loadRobot(const QString &hcrobot)
{
    RobotInstance* newInstance = new RobotInstance(this);
    loadObject(newInstance,QVariant(hcrobot));
}

void occQt::loadObject(SceneObject* newObject,const QVariant& data)
{
    if(!newObject) return;
    bool success = newObject->load(myOccView->getContext(),data);

    if(success){
//        // 1. 使用 dynamic_cast 检查传入的对象是否是一个 RobotInstance
//        RobotInstance* newRobot = dynamic_cast<RobotInstance*>(newObject);
//        // 2. 如果转换成功（即加载的是一个机器人），则建立连接
//        if (newRobot) {
//            connect(newRobot, &RobotInstance::trajectoryPointCreated, myOccView, &OccView::createPoint);
//        }
        // --- 3. 【核心修改】智能命名逻辑 ---
        QString id = newObject->getId(); // a. 首先获取对象可能已经携带的ID

        // b. 只有当对象没有预设ID，或者ID是一个通用的基础名时，我们才为它生成新ID
        if (id.isEmpty() || id == "Trajectory" || id == "Table" || id == "DynamicCube") {
            QString baseName = newObject->getBaseName();
            id = baseName;
            int counter = 1;
            while (m_sceneObjects.contains(id)) {
                id = QString("%1_%2").arg(baseName).arg(counter);
                counter++;
            }
            newObject->setId(id); // 将生成的新ID设置回去
        }
        // c. 如果对象已有预设的、有意义的ID（如"KUKA_traj_1"），则上面的if不执行，直接使用该ID
        // 6. 将新的、加载成功的实例添加到管理器中
        m_sceneObjects.insert(id, newObject);

        // --- 新增代码：将新机器人添加到模型树中 ---
        QStandardItem* objectItem = new QStandardItem(id); // 使用机器人ID作为节点名
        objectItem->setEditable(false); // 设置为不可编辑
        // 根据对象类型设置不同的图标
        if (dynamic_cast<RobotInstance*>(newObject)) {
            objectItem->setIcon(QIcon(":/Resources/robot.png"));
        } else if (dynamic_cast<Trajectory*>(newObject)) {
            objectItem->setIcon(QIcon(":/Resources/trajectory.png")); // 假设你有一个轨迹的图标
        } else {
            objectItem->setIcon(QIcon(":/Resources/cube.png")); // 其他物体的图标
        }
        molding_tree_view_model.appendRow(objectItem); // 添加到模型树的顶层

        // 7. 将新加载的机器人设置为当前活动实例 (此方法将在下一步实现)
        setActiveObject(id);


        // 8. 更新应用级别的UI和设置 (这些是管理者occQt的职责，予以保留)
        //    只有加载机器人时，才显示“加载模型成功”和保存路径
        if (dynamic_cast<RobotInstance*>(newObject)) {
            ui.statusBar->showMessage("加载模型: " + id + " 成功", 5000);
            // 注意：data是QVariant，需要转换为QString
            if (data.canConvert<QString>()) {
                 SystemSetting::Instance()->setCustomSetting("last_hcrobot", data.toString());
            }
            myOccView->fitAll();
        }
//        ui.statusBar->showMessage("加载模型: " + id + " 成功", 5000);
//        SystemSetting::Instance()->setCustomSetting("last_hcrobot", data);
//        myOccView->fitAll();
        // 动态更新广播按钮的可用状态 (判断机器人数量)
                int robotCount = 0;
                for(SceneObject* obj : m_sceneObjects) {
                    if(dynamic_cast<RobotInstance*>(obj)) {
                        robotCount++;
                    }
                }
                m_broadcastAction->setEnabled(robotCount > 1);

    } else {
        // 如果加载失败，清理创建失败的实例，防止内存泄漏
        delete newObject;
        // 详细的错误信息已在 RobotInstance::load 内部通过 QMessageBox 显示
    }
}


void occQt::setActiveObject(const QString& objectId)
{
    // 1. 验证并查找实例
    if (!m_sceneObjects.contains(objectId)) {
        m_activeObject = nullptr;
        // 如果找不到实例 (例如，在删除最后一个机器人后)，则禁用UI面板
        if (robot_jonits) robot_jonits->setEnabled(false);
        if (robot_base_tool) robot_base_tool->setEnabled(false);
        robot_name->setText("活动模型: 无");
        //当没有活动模型是，禁用 移除 按钮
        m_removeAction->setEnabled(false);
        return;
    }

    // 2. 设置当前活动实例指针
    m_activeObject = m_sceneObjects.value(objectId);

    // 启用UI面板
    if (robot_jonits) robot_jonits->setEnabled(true);
    if (robot_base_tool) robot_base_tool->setEnabled(true);
    //当有活动模型是，启用 移除 按钮
    m_removeAction->setEnabled(true);

    // ----- 开始：逻辑严格迁移自原 occQt::loadRobot 成功后的UI设置部分 -----

    // 3.只有当前活动是机器人才更新机器人相关的UI
    RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
    if(activeRobot){
        RobotConfig* config = activeRobot->getConfig();
        QVector<double> currentJoints = activeRobot->getCurrentJointValues();

        // 4. 使用获取的信息，动态配置 robot_jonits 面板
        if (robot_jonits && config) {
            // a. 设置关节数量
            robot_jonits->setAxisNum(config->jointNumber());

            // b. 循环为每个关节设置运动范围
            for (int i = 0; i < config->jointNumber(); ++i) {
                JointInfo info = config->getJointInfo(i);
                robot_jonits->setJointRange(i, info.p_limit, info.n_limit, 3);
            }

            // c. 将UI滑块的位置设置为机器人当前的实际关节角度
            robot_jonits->setJointsValue(currentJoints);
        }

        // 5. TODO: 以类似方式配置 robot_base_tool 面板的初始值
        // 例如: robot_base_tool->setBaseJoints(m_activeInstance->getBasePose());

        // 6. 更新状态栏，显示当前活动机器人的名称
        robot_name->setText("活动模型: " + activeRobot->getId());

        //同步记录轨迹按钮的勾选状态
        if(activeRobot->isRecording()){ // <-- 替换为 isNewRecording()
            ui.actionStartrecord->setChecked(false); // 正在录，所以“开始”不勾选
            ui.actionStoprecord->setChecked(true);  // “停止”按钮应被勾选
        }else{
            ui.actionStartrecord->setChecked(true);   // 没在录，“开始”按钮应被勾选
            ui.actionStoprecord->setChecked(false);
        }
    }else{
        // 如果选中的不是机器人（比如是一个桌子），则禁用关节控制面板
        if (robot_jonits) robot_jonits->setEnabled(false);
        if (robot_base_tool) robot_base_tool->setEnabled(false);
        robot_name->setText("活动对象: " + m_activeObject->getId());
    }
    // ----- 结束：逻辑严格迁移 -----
    updateActionStates();
}
/**
 * @brief 槽函数：响应关节控制面板变化的槽函数
 * @param joints 来自robot_jonits UI面板的新的关节角度值
 */
void occQt::onActiveRobotJointsChanged(QVector<double> joints)
{
    // 广播模式
    if(m_controlMode == ControlBroadcast){
        for(SceneObject* obj : m_sceneObjects.values()){
            RobotInstance* instance = dynamic_cast<RobotInstance*>(obj);
            if (instance) { // 确保只对机器人操作
                instance->updateJoints(joints);
            }
        }
    } else { // 单体模式
        RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
        if(activeRobot){
            activeRobot->updateJoints(joints);
        }
    }
}
/**
 * @brief 槽函数：响应基座更新的槽函数
 * @param pos 来自robot_base_tool UI面板的新的基座位姿
 */
void occQt::onActiveRobotBaseChanged(const QVector<double>& pos)
{
   RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
   if (activeRobot) { // <-- 修改
       activeRobot->updateBase(pos); // <-- 修改
   }
}
/**
 * @brief 槽函数：响应工具更新的槽函数
 * @param pos 来自robot_base_tool UI面板的新的工具位姿
 */
void occQt::onActiveRobotToolChanged(const QVector<double>& pos)
{
    RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
    if (activeRobot) { // <-- 修改
        activeRobot->updateTool(pos); // <-- 修改
    }
}



//全局更新定时器的槽函数，应用所有运动学变换并刷新视图
void occQt::onUpdateRequest()
{
    // 1. 更新所有已加载的机器人实例的3D位姿
    for(SceneObject *object : m_sceneObjects.values()){
        object->update();// 更新逻辑状态
    }
   // 3. 所有计算完成后，统一重绘视图一次，确保画面同步更新
    myOccView->getView()->Redraw();

//**讲解**: `dynamic_cast` 是实现这一步的关键。它能在运行时安全地检查一个基类指针是否真的指向一个特定的子类对象。这是我们调用机器人特有功能（如 `getCurrentTcpPos`）之前的“安全阀”。
}

// --- 新增：添加槽函数的完整实现 ---
void occQt::onTreeViewSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous); // 我们不关心之前选中的是哪个，所以标记为未使用

    if (!current.isValid()) {
        return; // 如果点击的是空白区域，索引无效，则直接返回
    }

    // 1. 从模型索引获取被点击的 QStandardItem
    const QStandardItem* item = molding_tree_view_model.itemFromIndex(current);
    if (!item) {
        return;
    }

    // 2. 找到该项目所属的顶层项目 (因为用户可能点击的是子节点)
    const QStandardItem* topLevelItem = item;
    while (topLevelItem->parent()) {
        topLevelItem = topLevelItem->parent();
    }

    // 3. 获取顶层项目的文本，也就是我们的机器人ID
    QString objectId = topLevelItem->text();

    // 4. 检查切换是否必要，并调用setActiveInstance
    // (如果当前活动实例不是被点击的这个，或者当前没有活动实例)
    if (!m_activeObject || m_activeObject->getId() != objectId) {
        setActiveObject(objectId);
        ui.statusBar->showMessage("已切换活动模型为: " + objectId, 3000);
    }

    // --- 新增：同步更新活动轨迹指针 ---
    // 1. 尝试在 m_trajectories 中查找该ID
    if (m_trajectories.contains(objectId)) {
        // 2. 如果找到了，说明用户选中的就是一个 Trajectory 对象，将其设为活动轨迹
        m_activeTrajectory = m_trajectories.value(objectId);
    } else {
        // 3. 如果没找到，说明用户选中的是机器人或其他物体，则清空活动轨迹指针
        m_activeTrajectory = nullptr;
    }
    updateActionStates();

    // 临时调试代码
//    RobotInstance* robot = dynamic_cast<RobotInstance*>(m_activeObject);
//    if (robot) {
//        QList<TrackableLink> links = robot->getTrackableLinks();
//        qDebug() << "Trackable links for" << robot->getId() << ":";
//        for (const auto& link : links) {
//            qDebug() << "  -" << link.id;
//        }
//    }
}

void occQt::robotComunicationEnable(bool status)
{
    if(status){
        if(SystemSetting::Instance()->getUdpNetEnable()){
            robot_communication->initUdp(SystemSetting::Instance()->getNetworkPort(),SystemSetting::Instance()->getNetworkComFormat());
            auto_connect.stop();
        }
        else{
            robot_communication->connect(SystemSetting::Instance()->getTcpNetIpAddr(),SystemSetting::Instance()->getTcpNetPort());
            robot_communication->startThread();
            auto_connect.start(1000*SystemSetting::Instance()->getNetworkAutoConnectTime());
        }
        network_status->setText("网络通讯:开启");
        SystemSetting::Instance()->setLastNetworkState(true);
    }
    else{
        robot_communication->closeUdp();
        robot_communication->disconnect();
        auto_connect.stop();
        network_status->setText("网络通讯:关闭");
        SystemSetting::Instance()->setLastNetworkState(false);
    }
}

void occQt::onBroadcastToggled(bool checked)
{
    if(checked){
        m_controlMode = ControlBroadcast;
        m_controlModeLabel->setText("控制模式：广播");
        ui.statusBar->showMessage("广播模式已开启：所有模型将同步运行");
    }else{
        m_controlMode = ControlSingle;
        m_controlModeLabel->setText("控制模式：单体");
        ui.statusBar->showMessage("广播模式已关闭：仅控制活动模型");
    }
}

void occQt::onRemoveActiveObject()
{
    // 1. 确认当前确实有一个活动实例
    if(!m_activeObject){
        return;
    }
    // 2. 弹出对话框，防止误操作
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"确认移除",
                                  QString("你确认移除'%1'吗？").arg(m_activeObject->getId()),
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
        removeObject(m_activeObject->getId());
}

void occQt::removeObject(const QString &objectId)
{
    if(!m_sceneObjects.contains(objectId))
        return;
    //按照严谨的顺序执行清理工作
    //1.从模型树UI中移除节点
    for(int i = 0;i<molding_tree_view_model.rowCount();++i){
        if(molding_tree_view_model.item(i)->text() == objectId){
            molding_tree_view_model.removeRow(i);
            break;
        }
    }
    //2.从管理器中获取实例指针
    SceneObject* objectToRemove = m_sceneObjects.value(objectId);

    //3.从3D场景中移除所有相关的集合体
    if (objectToRemove) {
        objectToRemove->removeFromScene(myOccView->getContext());
    }
    //4.从管理器QMap中移除记录
    m_sceneObjects.remove(objectId);
      //从 m_trajectories 中移除记录
    if(m_trajectories.contains(objectId)){
        m_trajectories.remove(objectId);
    }
    //5.检查被删除的是否是活动实例，并处理后续状态
    if(m_activeObject == objectToRemove){
        //如果是空，则清楚活动实例指针
        m_activeObject = nullptr;
        m_activeTrajectory = nullptr; // 同时清空活动轨迹指针
        // 尝试激活另一个模型（比如列表第一个），并立即更新UI状态
        if(!m_sceneObjects.isEmpty()){
            // 注意：此时 objectToRemove 还在 m_sceneObjects 中，先临时移除
            m_sceneObjects.remove(objectId);
            if (!m_sceneObjects.isEmpty()) {
                 setActiveObject(m_sceneObjects.firstKey());
            } else {
                 setActiveObject(""); // 最后一个对象被删除了
            }
            // 把要删除的对象再加回来，以便后续的 delete
            m_sceneObjects.insert(objectId, objectToRemove);
        } else {
            setActiveObject(""); // 最后一个对象被删除了
        }
    }

    RobotInstance* robotToRemove = dynamic_cast<RobotInstance*>(objectToRemove);
    if (robotToRemove) {
        // 如果删除的是机器人，找到它创建的所有轨迹并一并删除
        QList<Trajectory*> trajectoriesToRemove;
        for(const auto& task : robotToRemove->getRecordingTasks()) { // 假设RobotInstance有getRecordingTasks()
            trajectoriesToRemove.append(task.targetTrajectory);
        }
        for(Trajectory* traj : trajectoriesToRemove) {
            removeObject(traj->getId()); // 递归调用removeObject来删除轨迹
        }
    }
    //6.最后，安全的释放实例占用的内存
    delete objectToRemove;
    //7. 重新计算机器人数量来决定是否启用广播
        int robotCount = 0;
        for(SceneObject* obj : m_sceneObjects) {
            if(dynamic_cast<RobotInstance*>(obj)) {
                robotCount++;
            }
        }
        m_broadcastAction->setEnabled(robotCount > 1);

        ui.statusBar->showMessage("模型"+objectId+"已经移除",3000);
}

void occQt::updateActionStates()
{
    // --- 1. 获取当前状态 ---
    RobotInstance* activeRobot = dynamic_cast<RobotInstance*>(m_activeObject);
    Trajectory* activeTrajectory = m_activeTrajectory; // m_activeTrajectory 已由 onTreeViewSelectionChanged 更新

    // --- 2. 重置所有上下文相关 Action 的状态为默认（禁用/未勾选）---
    ui.actionStartrecord->setEnabled(false);
    ui.actionStartrecord->setChecked(false);
    ui.actionStoprecord->setEnabled(false);
    ui.actionStoprecord->setChecked(false);

    ui.actionCleartrajectory->setEnabled(false);
    ui.actionExporttrajectory->setEnabled(false);

    // --- 精确地禁用/启用颜色按钮 ---
    for (QAction* action : trajectory_tool_bar->actions()) {
        // 我们通过检查 action 的 text() 来识别，这与你 createToolBars 中的代码一致
        QString text = action->text();
        if (text == "轨迹颜色" || text == "红" || text == "绿" || text == "黄" || text == "蓝") {
            action->setEnabled(false); // 默认禁用
        }
    }
    ui.actionCleartrajectory->setEnabled(false);
    ui.actionExporttrajectory->setEnabled(false);


    // --- 3. 根据当前选择，重新激活对应的 Action ---

    // 情况一：如果当前选中的是一个机器人
    if (activeRobot) {
        if (activeRobot->isRecording()) {
            // a) 如果这个机器人正在录制
            ui.actionStartrecord->setEnabled(false); // 正在录，不能再开始
            ui.actionStoprecord->setEnabled(true);   // 可以停止
            ui.actionStoprecord->setChecked(true);   // 保持勾选状态
        } else {
            // b) 如果这个机器人没有在录制
            ui.actionStartrecord->setEnabled(true);  // 可以开始
            ui.actionStoprecord->setEnabled(false);  // 已经停了，不能再停止
            ui.actionStartrecord->setChecked(false); // 保持非勾选状态
        }
    }

    // 情况二：如果当前选中的是一个轨迹
    if (activeTrajectory) {
        ui.actionCleartrajectory->setEnabled(true);
        ui.actionExporttrajectory->setEnabled(true);

        // 启用所有颜色按钮
        for (QAction* action : trajectory_tool_bar->actions()) {
            QString text = action->text();
            if (text == "轨迹颜色" || text == "红" || text == "绿" || text == "黄" || text == "蓝") {
                action->setEnabled(true);
            }
        }
    }
}

void occQt::about()
{
    QMessageBox::about(this, tr("关于"),
                       this->windowTitle()+"\n"+
                           tr("深圳市华成工业控制股份有限公司\nwww.hc-system.com\n")+"\n\n第三方lib:\n"
                            "OpenCASCADE-7.3.0\n"
                            "Lua 5.3.0\n"
                            "QCustomPlot 2.1.1\n"
                            "QMQX QMQTT ");
}

//< 打开按钮
void occQt::open()
{
    QDir dir;
    dir.cd("robot");
    // --- 修改点1：使用 getOpenFileName (单数) 并指定 .hcrobot 文件类型 ---
    QString filePath = QFileDialog::getOpenFileName(nullptr, QObject::tr("打开华成机器人仿真工程"),
                                                        dir.absolutePath(), QObject::tr("华成机器人配置文件 (*.hcrobot)"));

    if (!filePath.isEmpty()) {
        loadRobot(filePath); // 调用加载单个配置文件的版本
    }
}





